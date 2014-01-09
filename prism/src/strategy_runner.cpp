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
		auto sp = trans.asset_indexer_.lock();
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

	void StrategyRunner::ForwardToCursor()
	{
		for (auto& ai : asset_indexer_list_)
			ai.ForwardTo(cursor_);
	}

	void StrategyRunner::Screen()
	{
		attach_screener_->Screen(asset_indexer_list_, &to_attach_, cursor_);
		detach_screener_->Screen(asset_indexer_list_, &to_detach_, cursor_);
	}

	void StrategyRunner::Trade()
	{

	}

	void StrategyRunner::Run()
	{
		runner_observer_->OnStart(strategy_.get());
		MoveToCursor();
		while (cursor_ < strategy_->end_time())
		{
			runner_observer_->OnCycleBegin(cursor_);
			
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