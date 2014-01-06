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
	
	void TransactionManager::GetTransactions(const std::string& symbol, TransactionList* symbol_transactions)
	{
		symbol_transactions->clear();
		for (auto t : transactions_)
		{
			auto sp = t.asset_indexer_.lock();
			if (sp)
			{
				if (sp->asset()->symbol() == symbol)
					symbol_transactions->push_back(t);
			}
		}
	}

	unsigned int TransactionManager::id = 0;

	bool Portfolio::GetValue(double& value, time_t pos)
	{
		auto sp = asset_indexer_.lock();
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
			value = hloc.close * amount_;
			return true;
		}
		else
		{
			return false;
		}
	}

	void PortfolioManager::Buy(const std::weak_ptr<AssetIndexer>& asset_indexer, double amount)
	{
		auto sp = asset_indexer.lock();
		if (sp)
		{
			auto cit = portfolios_.find(sp->asset()->symbol());
			if (cit != portfolios_.end())
			{
				cit->second->Buy(amount);
			}
			else
			{
				auto portfolio = std::make_unique<Portfolio>(asset_indexer, amount);
				portfolios_.insert(std::make_pair(sp->asset()->symbol(), std::move(portfolio)));
			}
		}
	}

	void PortfolioManager::Sell(const std::weak_ptr<AssetIndexer>& asset_indexer, double amount)
	{
		auto sp = asset_indexer.lock();
		if (sp)
		{
			auto cit = portfolios_.find(sp->asset()->symbol());
			if (cit != portfolios_.end())
			{
				cit->second->Sell(amount);
			}
		}
	}

	void PortfolioManager::Clear()
	{
		portfolios_.clear();
	}

	bool PortfolioManager::GetValue(double& value, time_t pos)
	{
		value = 0.0;
		for (auto& it : portfolios_)
		{
			double portfolio_value;
			bool ret = it.second->GetValue(portfolio_value, pos);
			if (ret)
				value += portfolio_value;
			else
				return false;
		}
		return true;
	}

	Portfolio* PortfolioManager::Get(const std::string& symbol)
	{
		auto cit = portfolios_.find(symbol);
		return cit == portfolios_.end()? nullptr : cit->second.get();
	}

}