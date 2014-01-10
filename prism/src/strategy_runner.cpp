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
		int count = 0;
		if (reload)
		{
			count = assets_provider_->LoadAssets(strategy_->stocks(),
				GetYear(strategy_->begin_time()),
				GetYear(strategy_->end_time()));
		}
		if (count > 0)
		{
			InitIndexer();
			attach_screener_ = std::make_unique<Screener>(strategy_->attach_rule());
			detach_screener_ = std::make_unique<Screener>(strategy_->detach_rule());
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

	void StrategyRunner::Trade()
	{
		// sell stock if it is in the detach list..
		for (auto& i : to_detach_)
		{
			std::string symbol = asset_indexer_list_[i].asset()->symbol();
			if (portfolio_manager_->Size() > 0)
			{

			}
		}
		// buy stock if there is enough cash and the attach list is not empty..
		if (cash_ > 100 && to_attach_.size() > 0)
		{
			unsigned int num_to_buy = strategy_->num_portfolios() - portfolio_manager_->Size();
			num_to_buy = num_to_buy < to_attach_.size() ? num_to_buy : to_attach_.size();
			if (num_to_buy > 0)
			{
				// could buy some stocks..
				srand((unsigned int)time(NULL));
				double money = cash_ / num_to_buy;
				for (unsigned int i = 0; i < num_to_buy; ++i)
				{
					// random select to buy in the to attach list
					int k = rand() % to_attach_.size();
					AssetIndexer *asset_indexer = &asset_indexer_list_[to_attach_[k]];
					if (asset_indexer->GetIndexTime() == cursor_)
					{
						size_t index = asset_indexer->index();
						HLOC hloc;
						bool ret = asset_indexer->GetIndexData(hloc);
						if (!ret)
							break;
						double price = hloc.open;
						int amount_hands = (int)(money / (kHand * (1 + kCommissionRate)*price));
						if (amount_hands > 0)
						{
							double shares = 100 * amount_hands;
							money = shares * price;
							Transaction trans;
							trans.asset_indexer_ = asset_indexer;
							trans.price_ = price;
							trans.shares_ = shares;
							trans.time_ = asset_indexer->GetIndexTime();
							trans.type_ = TRANSACTION_TYPE_BUY;
							trans.commission_ = kCommissionRate * money;
							cash_ = cash_ - money - trans.commission_;
							if (runner_observer_)
								runner_observer_->OnTransaction(trans);
						}
					}
				}
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