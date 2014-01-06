// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for strategy runner


#ifndef STRATEGY_RUNNER_H
#define STRATEGY_RUNNER_H

#include "common.h"
#include "util.h"
#include "time.h"
#include "asset.h"
#include "trade.h"
#include "strategy.h"
#include <string>
#include <vector>
#include <map>

namespace prism {
		
	// the environment for strategy to run
	class StrategyRunner
	{
	public:
		StrategyRunner(const std::shared_ptr<Strategy>& strategy,
			const std::shared_ptr<AssetsProvider>& assets_provider,
			const std::shared_ptr<PortfolioManager>& portfolio_manager,
			const std::shared_ptr<TransactionManager>& transaction_manager){}
		~StrategyRunner(){}
	};

	class StrategyObserver
	{
	public:
		StrategyObserver(StrategyRunner* runner);
		~StrategyObserver();
	public:
		void OnTransaction(const Transaction& trans);
		void OnStart();
		void OnCycleBegin(time_t cursor);
		void OnCycleEnd(time_t cursor);
		void OnFinished();
		int GetTransactionsNum(bool win = true);
	public:
		DoubleTimeList& performance_series() { return performance_series_; }
		std::vector<Transaction>& transactions() { return transactions_; }
		double profit() { return profit_; }
	private:
		DoubleTimeList performance_series_;
		std::vector<Transaction> transactions_;		
		StrategyRunner* runner_;
		double profit_;
	};

}

#endif