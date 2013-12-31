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
		transacitons_.push_back(trans); 
	}
	
	void TransactionManager::GetTransactions(const std::string& symbol, TransactionList* symbol_transactions)
	{
		symbol_transactions->clear();
		for (auto t : transacitons_)
		{
			if (t.asset_indexer_->asset()->symbol() == symbol)
				symbol_transactions->push_back(t);
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

	void PortfolioManager::Buy(AssetIndexer* asset_indexer, double amount)
	{
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			cit->second->Buy(amount);
		}
		else
		{
			auto portfolio = new Portfolio(asset_indexer, amount);
			portfolios_.insert(std::make_pair(asset_indexer->asset()->symbol(), portfolio));
		}
	}

	void PortfolioManager::Sell(AssetIndexer* asset_indexer, double amount)
	{
		auto cit = portfolios_.find(asset_indexer->asset()->symbol());
		if (cit != portfolios_.end())
		{
			cit->second->Sell(amount);
		}
	}

	void PortfolioManager::Clear()
	{
		for (auto it : portfolios_)
			delete it.second;
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

	Portfolio* PortfolioManager::Get(const std::string& symbol)
	{
		auto cit = portfolios_.find(symbol);
		return cit == portfolios_.end() ? nullptr : cit->second;
	}

}