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
			auto sp = t.asset_indexer_;
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
		auto sp = asset_indexer_;
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

	double PortfolioManager::Buy(AssetIndexer* asset_indexer, double amount)
	{
		double value = 0.0F;
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			double prev;
			cit->second->GetValue(prev);
			cit->second->Buy(amount);
			cit->second->GetValue(value);
			value = value - prev;
		}
		else
		{
			auto portfolio = std::make_unique<Portfolio>(asset_indexer, amount);
			portfolios_.insert(std::make_pair(asset_indexer->asset()->symbol(), std::move(portfolio)));
			portfolio->GetValue(value);
		}

		return value * (1 + kCommissionRate);
	}

	double PortfolioManager::Sell(AssetIndexer* asset_indexer, double amount)
	{
		double value = 0.0F;
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			double prev;
			cit->second->GetValue(prev);
			cit->second->Sell(amount);
			cit->second->GetValue(value);
			value = prev - value;
		}
		return value * (1 - kCommissionRate);
	}

	double PortfolioManager::SellAll(AssetIndexer* asset_indexer)
	{
		double value = 0.0F;
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			cit->second->GetValue(value);
			portfolios_.erase(cit);
		}
		return value * (1 - kCommissionRate);
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