// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// for strategy screener implementation.

#include "strategy_screener.h"
#include "store.h"
#include "asset.h"
#include "rule.h"
// kc utils
#include <kcutil.h>

namespace prism {
	
	StrategyScreener::StrategyScreener(std::shared_ptr<IStore> store, const std::string& strategy_file) : store_(store), strategy_file_(strategy_file)
	{
		runner_ = new StrategyRunner(store_);
		observer_ = new StrategyObserver(runner_);
		runner_->set_observer(observer_);
		//std::cout << "StrategyScreener::StrategyScreener()" << std::endl;

	}

	StrategyScreener::~StrategyScreener()
	{
		delete observer_;
		delete runner_;
		//std::cout << "StrategyScreener::~StrategyScreener()" << std::endl;

	}

	void StrategyScreener::SetStrategyParam(Strategy* strategy)
	{
	}

	bool StrategyScreener::Test()
	{
		bool ret = runner_->Load(strategy_file_);
		if (!ret)
			return false;

		SetStrategyParam(runner_->strategy());

		std::vector<std::string> symbols;
		GetSymbols(symbols);
		std::vector<std::string>::const_iterator cit = symbols.begin();
		while (cit != symbols.end())
		{
			if (TestOnSymbol(*cit))
				cit++;
			else
				return false;
		}
		return true;
	}

	bool StrategyScreener::TestOnSymbol(const std::string& symbol)
	{
		// set the symbol
		std::string stocks = "patterns=" + symbol;
		runner_->strategy()->set_stocks(stocks);
		// run backtest against the symbol
		if (!runner_->Initialize())
			return false;
		runner_->Run();
		// get performance
		StrategyPerformance perf;
		int num_trans = observer_->transactions().size();
		if (num_trans > 0)
		{
			perf.strategy_profit_ = observer_->profit();
			perf.symbol_ = symbol;
			perf.begin_time_ = runner_->strategy()->begin_time();
			perf.end_time_ = runner_->strategy()->end_time();
			perf.num_transactions_ = num_trans;
			perf.win_transactions_ = observer_->GetTransactionsNum(true);
			perf.win_rate_ = perf.win_transactions_ * 2.0 * 100 / (double)perf.num_transactions_;
			
			HLOCList* raw_data = observer_->transactions().at(0).asset_->raw_data().get();
			size_t num_points = raw_data->size();
			size_t i = 0;
			while (i < num_points)
			{
				if (raw_data->at(i).time < perf.begin_time_)
					i++;
				else
					break;
			}
			double first_price = raw_data->at(i).close;
			//std::cout << perf.symbol_ << "," << TimeToString(raw_data->at(i).time, "first: %Y-%m-%d") <<", price:"<< first_price << std::endl;

			while (i < num_points)
			{
				if (raw_data->at(i).time < perf.end_time_)
					i++;
				else
					break;
			}
			i = i < num_points ? i : num_points - 1;
			double last_price = raw_data->at(i).close;
			//std::cout << TimeToString(raw_data->at(i).time, "last: %Y-%m-%d") << ", price:" << last_price << std::endl;

			perf.original_profit_ = (last_price - first_price) * 100 / first_price; 
			perf.profit_rate_ = perf.strategy_profit_ - perf.original_profit_;
			performance_.push_back(perf);
		}
		// clearup
		runner_->CleanUp();
		return true;
	}

	void StrategyScreener::GetSymbols(std::vector<std::string>& symbols)
	{
		std::string symbols_str;
		symbols.clear();
		if (store_->GetBlock(kBlockAllAShares, symbols_str))
		{
			kyotocabinet::strsplit(symbols_str, '\n', &symbols);
		}
	}

	int StrategyScreener::GetPositiveNum()
	{
		struct positive_counter {
			bool operator ()(const StrategyPerformance& perf) const {
				return perf.profit_rate_ > 0;
			}
		};
		int count = std::count_if(performance_.begin(), performance_.end(), positive_counter());
		return count;
	}
	
	void StrategyScreener::DumpResult(const std::string& out_file)
	{
		struct perf_compare {
			bool operator ()(const StrategyPerformance& a, const StrategyPerformance& b) const {
				return a.profit_rate_ > b.profit_rate_;
			}
		};
		std::sort(performance_.begin(), performance_.end(), perf_compare());

		std::ofstream of;
		of.open(out_file, std::ios::out);
		size_t count = performance_.size();
		for (size_t i = 0; i < count; ++i)
		{
			StrategyPerformance perf = performance_.at(i);
			of << perf.symbol_ << "," << perf.num_transactions_ << "," << perf.win_transactions_ << "," 
				<< perf.win_rate_ << "%," <<perf.strategy_profit_ << "%" << ","
				<< perf.original_profit_ << "%," << perf.profit_rate_ << "%" << std::endl;
		}
		of.close();
	}

	MACDStrategyScreener::MACDStrategyScreener(std::shared_ptr<IStore> store, 
		const std::string& strategy_file, 
		int short_period, 
		int long_period, 
		int signal_period, 
		bool linear_predict, 
		int look_back, 
		double threshold): StrategyScreener(store, strategy_file),
		short_period_(short_period), long_period_(long_period), signal_period_(signal_period), \
		linear_predict_(linear_predict), look_back_(look_back), threshold_(threshold)
	{
		//std::cout << "MACDStrategyScreener::MACDStrategyScreener()" << std::endl;

	}

	MACDStrategyScreener::~MACDStrategyScreener()
	{
		//std::cout << "MACDStrategyScreener::~MACDStrategyScreener()" << std::endl;
	}

	void MACDStrategyScreener::SetStrategyParam(Strategy* strategy)
	{
		assert(strategy->screen_rule());
		IRule* rule = strategy->screen_rule();
		assert(rule->type() == RULE_TYPE_INDICATOR_MACD);
		MACDRule* macd_rule = (MACDRule*)rule;
		macd_rule->set_short_period(short_period_);
		macd_rule->set_long_period(long_period_);
		macd_rule->set_signal_period(signal_period_);
		macd_rule->set_linear_predict(linear_predict_);
		macd_rule->set_look_back(look_back_);
		macd_rule->set_threshold(threshold_);

	}


}