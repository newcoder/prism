// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// for trade implementation.

#include "trade.h"
#include <ctime>
#include <assert.h>
#include <set>

namespace prism {
	
	void TransactionManager::Add(Transaction& trans) 
	{
		trans.transaction_id_ = TransactionManager::id++;
		transactions_.push_back(trans);
	}

	Transaction* TransactionManager::Get(int id)
	{
		for (auto& t : transactions_)
		{
			if (t.transaction_id_ == id)
			{
				return &t;
			}
		}
		return nullptr;
	}
	
	void TransactionManager::GetTransactions(const std::string& symbol, TransactionList* symbol_transactions)
	{
		symbol_transactions->clear();
		for (auto t : transactions_)
		{
			if (t.asset_indexer_->asset()->symbol() == symbol)
				symbol_transactions->push_back(t);
		}
	}

	unsigned int TransactionManager::id = 0;

	bool Portfolio::GetValue(double& value, time_t pos)
	{
		auto sp = trans_.asset_indexer_;
		if (sp)
		{
			if (pos > 0)
			{
				sp->ToBegin();
				sp->ForwardTo(pos);
			}
			if (!sp->valid())
				return false;
			HLOC hloc;
			bool ret = sp->GetIndexData(hloc);
			if (!ret) return false;
			value = hloc.close * trans_.shares_;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool PortfolioManager::GetValue(double& value, time_t pos)
	{
		value = 0.0;
		for (auto& it : portfolios_)
		{
			double portfolio_value;
			bool ret = it.GetValue(portfolio_value, pos);
			if (ret)
				value += portfolio_value;
			else
				return false;
		}
		return true;
	}

	void PortfolioManager::Remove(int transaction_id)
	{
		auto it = portfolios_.begin();
		while (it != portfolios_.end())
		{
			if ((*it).transaction_id() == transaction_id)
			{
				portfolios_.erase(it);
				break;
			}
		}
	}

	void PortfolioManager::GetPortfolios(const std::string& symbol, PortfolioList* portfolios)
	{
		portfolios->clear();
		for (auto& it : portfolios_)
		{
			if (it.symbol() == symbol)
			{
				portfolios->push_back(it);
			}
		}
	}

	void CashBox::Init()
	{
		double amount_for_each = total_ / num_slots_;
		for (int i = 0; i < num_slots_; i++)
			slots_.push_back(amount_for_each);
	}

	bool CashBox::Get(double &amount)
	{
		for (int i = 0; i < num_slots_; i++)
		{
			if (slots_[i] > 0)
			{
				amount = slots_[i];
				slots_[i] = -1;
				return true;
			}
		}
		return false;
	}

	void CashBox::Put(double amount)
	{
		for (int i = 0; i < num_slots_; i++)
		{
			if (slots_[i] < 0)
			{
				slots_[i] = amount;
				break;
			}
		}
	}

}