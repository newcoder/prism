// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// for strategy implementation.

#include "strategy_runner.h"
#include "rule.h"
#include "store.h"
#include <ctime>
#include <assert.h>
#include <set>
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"

// kc utils
#include <kcutil.h>

namespace prism {
	
	RunnerObserver::RunnerObserver()
	{
	}

	RunnerObserver::~RunnerObserver()
	{
	}

	void RunnerObserver::OnTransaction(const Transaction& trans)
	{
		auto sp = trans.asset_indexer_;
		if (sp)
		{
			std::cout << trans.type_ << ", "
				<< sp->asset()->symbol() << ", "
				<< TimeToString(trans.time_, "%Y-%m-%d, ")
				<< trans.price_ << ", "
				<< trans.shares_ << std::endl;
		}
	}

	void RunnerObserver::OnStart(Strategy* strategy)
	{
		std::cout << TimeToString(strategy->begin_time(), "begin time: %Y-%m-%d") << std::endl;
		std::cout << TimeToString(strategy->end_time(), "end time: %Y-%m-%d") << std::endl;
		std::cout << "stocks: " << strategy->stocks() << std::endl;
		Clear();
	}

	void RunnerObserver::OnCycleBegin(time_t cursor)
	{
		std::cout << TimeToString(cursor, "cursor: %Y-%m-%d") << std::endl;
	}
	
	void RunnerObserver::OnCycleEnd(time_t cursor)
	{		

	}

	void RunnerObserver::OnFinished()
	{

	}


	bool StrategyRunner::Init(bool reload)
	{
		cash_ = strategy_->init_cash();
		cursor_ = strategy_->begin_time();
		Clear();
		int count = assets_provider_->assets().size();
		if (reload)
		{
			count = assets_provider_->LoadAssets(strategy_->stocks(),
				GetYear(strategy_->begin_time()),
				GetYear(strategy_->end_time()));
		}
		if (count > 0)
		{
			InitIndexer();
			attach_screener_->set_rule(strategy_->attach_rule());
			detach_screener_->set_rule(strategy_->detach_rule());
			// put all init cash to the box
			cash_box_->Init(cash_, strategy_->num_portfolios());
			cash_ = 0.0F;
			return true;
		}
		return false;
	}

	void StrategyRunner::InitIndexer()
	{
		auto assets = assets_provider_->assets();
		for (auto& a : assets)
		{
			asset_indexer_list_.push_back(AssetIndexer(a.second));
		}
	}

	void StrategyRunner::MoveToCursor()
	{
		for (auto& ai : asset_indexer_list_)
			ai.MoveTo(cursor_);
	}

	inline void StrategyRunner::ForwardToCursor()
	{
		for (auto& ai : asset_indexer_list_)
			ai.ForwardTo(cursor_);
	}

	inline void StrategyRunner::Screen()
	{
		attach_screener_->Screen(asset_indexer_list_, &to_attach_, cursor_);
		detach_screener_->Screen(asset_indexer_list_, &to_detach_, cursor_);
	}

	void StrategyRunner::Detach(const std::string& symbol)
	{
		PortfolioList portfolios;
		portfolio_manager_->GetPortfolios(symbol, &portfolios);
		if (portfolios.empty())
			return;
		for (auto& p : portfolios)
		{
			auto index_time = p.asset_indexer()->GetIndexTime();
			if (index_time != cursor_) // not trading on cursor_
				return;
			HLOC hloc;
			bool ret = p.asset_indexer()->GetIndexData(hloc);
			assert(ret);
			if (!p.asset_indexer()->within_limit())
				break;
			double price = hloc.open;
			Transaction trans;
			trans.type_ = TRANSACTION_TYPE_SELL;
			trans.shares_ = p.shares();
			trans.price_ = price;
			trans.associate_transaction_id = p.transaction_id();
			trans.asset_indexer_ = p.asset_indexer();
			trans.time_ = p.asset_indexer()->GetIndexTime();
			double money = trans.price_ * trans.shares_;
			trans.commission_ = money * kCommissionRate;
			// record the transaction
			transaction_manager_->Add(trans);
			// put the money into cash box
			cash_box_->Put(cash_ + money - trans.commission_);
			// update the portfolios manager
			portfolio_manager_->Remove(p.transaction_id());
			// notify the observer
			runner_observer_->OnTransaction(trans);		
		}
	}

	bool StrategyRunner::Attach(double money)
	{
		// for now, random select an asset to buy in the to_attach list
		// TODO: ranking the attach list later...
		srand((unsigned int)time(NULL));
		int k = rand() % to_attach_.size();
		AssetIndexer *asset_indexer = &asset_indexer_list_[to_attach_[k]];
		auto index_time = asset_indexer->GetIndexTime();
		if (index_time != cursor_)
		{
			// not trading on cursor_
			to_attach_.erase(to_attach_.begin() + k);
			return false;
		}
		HLOC hloc;
		bool ret = asset_indexer->GetIndexData(hloc);
		assert(ret);
		if (!asset_indexer->within_limit())
		{
			to_attach_.erase(to_attach_.begin() + k);
			return false;
		}
		double price = hloc.open;
		int amount_hands = (int)(money / (kHand * (1 + kCommissionRate)*price));
		if (amount_hands > 0)
		{
			double shares = 100 * amount_hands;
			Transaction trans;
			trans.type_ = TRANSACTION_TYPE_BUY;
			trans.asset_indexer_ = asset_indexer;
			trans.price_ = price;
			trans.shares_ = shares;
			trans.time_ = asset_indexer->GetIndexTime();
			trans.commission_ = kCommissionRate * shares * price;
			trans.associate_transaction_id = -1; // no associate id for buy transaction 
			// put the remaining to cash_
			double remaining = money - trans.commission_ - shares * price;
			cash_ += remaining;
			// record the transaction
			transaction_manager_->Add(trans);
			// update the portfolios manager
			portfolio_manager_->Add(Portfolio(trans));
			// notify the observer
			runner_observer_->OnTransaction(trans);		
		}
		else
		{
			// money is not enough to buy one hand, put to cash_
			cash_ += money;
		}
		return true;
	}

	void StrategyRunner::Trade()
	{
		// detach stock if it is in the to_detach list..
		for (auto i : to_detach_)
		{
			std::string symbol = asset_indexer_list_[i].asset()->symbol();
			if (portfolio_manager_->Size() > 0)
			{
				Detach(symbol);
			}
		}

		// attach stock if the to_attach list is not empty..and there is money in the cash box
		if (to_attach_.size() > 0)
		{
			double money = -1;
			while (to_attach_.size() > 0 && cash_box_->Get(money))
			{
				if (!Attach(money))
					cash_box_->Put(money);
			}
		}
	}

	void StrategyRunner::Run()
	{
		runner_observer_->OnStart(strategy_.get());
		MoveToCursor();
		while (cursor_ < strategy_->end_time())
		{
			runner_observer_->OnCycleBegin(cursor_);
			Screen();
			cursor_ = cursor_ + strategy_->step() * 24 * 3600;
			ForwardToCursor();
			Trade();
			runner_observer_->OnCycleEnd(cursor_);
		}
		runner_observer_->OnFinished();
	}

	void StrategyRunner::Clear()
	{
		asset_indexer_list_.clear();
		transaction_manager_->Clear();
		portfolio_manager_->Clear();
		runner_observer_->Clear();
	}
}