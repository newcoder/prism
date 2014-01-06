// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes to screen strategy..

#ifndef STRATEGY_SCREENER_H
#define STRATEGY_SCREENER_H

#include "strategy.h"
#include "strategy_runner.h"

namespace prism {
	
	class StrategyPerformance
	{
	public:
		std::string symbol_;
		time_t begin_time_;
		time_t end_time_;
		int num_transactions_;
		int win_transactions_;
		double win_rate_;
		double strategy_profit_;
		double original_profit_;
		double profit_rate_;
	};

	class StrategyScreener
	{
	public:
		StrategyScreener(std::shared_ptr<IStore> store, const std::string& strategy_file);
		virtual ~StrategyScreener();
	public:
		virtual void SetStrategyParam(Strategy* strategy);
		bool Test();
		int GetPositiveNum();
		void DumpResult(const std::string& out_file);
	private:
		bool TestOnSymbol(const std::string& symbol);		
		void GetSymbols(std::vector<std::string>& symbols);
	private:
		std::shared_ptr<IStore> store_;
		StrategyRunner* runner_;
		StrategyObserver* observer_;
		std::string strategy_file_;
		std::vector<StrategyPerformance> performance_;
	};

	class MACDStrategyScreener: public StrategyScreener
	{
	public:
		MACDStrategyScreener(std::shared_ptr<IStore> store, const std::string& strategy_file, 
			int short_period, int long_period, int signal_period, 
			bool linear_predict, int look_back, double threshold);
		~MACDStrategyScreener();
	public:
		virtual void SetStrategyParam(Strategy* strategy);
	private:
		int short_period_;
		int long_period_;
		int signal_period_;
		bool linear_predict_;
		int look_back_;
		double threshold_;
	};

}

#endif