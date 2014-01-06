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
	
	StrategyObserver::StrategyObserver(StrategyRunner* runner): runner_(runner)
	{
	}

	StrategyObserver::~StrategyObserver()
	{
	}

	void StrategyObserver::OnTransaction(const Transaction& trans)
	{
		transactions_.push_back(trans);
		//std::cout << trans.type_ << ", " 
		//	<< trans.asset_->symbol() << ", " 
		//	<< TimeToString(trans.time_, "%Y-%m-%d, ") 
		//	<< trans.price_ << ", " 
		//	<< trans.shares_<< std::endl;	
	}

	void StrategyObserver::OnStart()
	{
		transactions_.clear();
		performance_series_.clear();
	}

	void StrategyObserver::OnCycleBegin(time_t cursor)
	{
		//std::cout << TimeToString(cursor, "cursor: %Y-%m-%d") << std::endl;
	}
	
	void StrategyObserver::OnCycleEnd(time_t cursor)
	{		
		DoubleTimePoint p;
		p.position = cursor;
		//p.value = runner_->GetBalance();
		performance_series_.push_back(p);
	}

	void StrategyObserver::OnFinished()
	{
		//std::cout << "transactions: "<< transactions_.size() << std::endl;
		//std::cout << "wins: "<< GetTransactionsNum(true) << std::endl;
		//double balance = runner_->GetBalance();
		//double init = runner_->strategy()->init_cash();
		//profit_ = (balance - init) * 100 / init;
		//std::cout << "profit: " << profit_ << "%" << std::endl;
	}

	int StrategyObserver::GetTransactionsNum(bool win)
	{
		int num = 0;
		std::map<std::string, Transaction> symbol_trans;
		std::vector<Transaction>::const_iterator cit = transactions_.begin();
		while(cit != transactions_.end())
		{
			Transaction trans = *cit;
			int amount = (int)trans.shares_;
			//std::string key = trans.asset_->symbol() + std::to_string(static_cast<long long>(amount));		
			if (trans.type_ == TRANSACTION_TYPE_BUY)
			{
			//	symbol_trans.insert(std::make_pair(key, trans));
			}
			else
			{
				//std::map<std::string, Transaction>::iterator it = symbol_trans.find(key);
				//assert(it != symbol_trans.end());
				//if (it != symbol_trans.end())
				//{
				//	if ((it->second.price_ < trans.price_ && win) || (it->second.price_ > trans.price_ && !win))
				//		num++;
				//	symbol_trans.erase(it);
				//}
			}
			cit++;
		}
		return num;
	}

}