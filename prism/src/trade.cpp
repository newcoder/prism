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
	
	void TransactionManager::GetTransactions(const std::string& symbol, std::shared_ptr<TransactionList> symbol_transactions)
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
		if (pos > 0)
		{
			asset_indexer_->ToBegin();
			asset_indexer_->ForwardTo(pos);
		}
		if (!asset_indexer_->valid())
			return false;
		HLOC hloc;
		bool ret = asset_indexer_->GetIndexData(hloc);
		if (!ret) return false;
		value = hloc.close * amount_;
		return true;
	}

	void PortfolioManager::Buy(std::shared_ptr<AssetIndexer> asset_indexer, double amount)
	{
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			cit->second->Buy(amount);
		}
		else
		{
			auto portfolio = std::make_shared<Portfolio>(asset_indexer, amount);
			portfolios_.insert(std::make_pair(asset_indexer->asset()->symbol(), portfolio));
		}
	}

	void PortfolioManager::Sell(std::shared_ptr<AssetIndexer> asset_indexer, double amount)
	{
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			cit->second->Sell(amount);
		}
	}

	void PortfolioManager::Clear()
	{
		portfolios_.clear();
	}

	bool PortfolioManager::GetValue(double& value, time_t pos)
	{
		value = 0.0;
		for (auto it : portfolios_)
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

	std::shared_ptr<Portfolio> PortfolioManager::Get(const std::string& symbol)
	{
		auto cit = portfolios_.find(symbol);
		return cit == portfolios_.end() ? nullptr : cit->second;
	}

}