// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for strategy
// strategy is about execute actions based on a rule...

#ifndef STRATEGY_H
#define STRATEGY_H

#include "common.h"
#include "util.h"
#include "time.h"
#include "asset.h"
#include <string>
#include <vector>
#include <map>

namespace prism {
	
	class IRule;
	class IStore;
	class StrategyObserver;

	typedef enum
	{
		TRANSACTION_TYPE_BUY = 0,
		TRANSACTION_TYPE_SELL,
		TRANSACTION_TYPE_BUY_TO_COVER,
		TRANSACTION_TYPE_SELL_SHORT
	} TRANSACTION_TYPE;

	class Transaction
	{
	public:
		TRANSACTION_TYPE type_;
		Asset* asset_;
		double price_;
		double shares_;
		double commission_;
		time_t time_;
	};

	class PortfolioItem
	{
	public:
		PortfolioItem(Asset* asset, double amount): asset_(asset), amount_(amount) {}
		Asset* asset() const { return asset_; }
		double amount() const { return amount_; }
	private:
		Asset* asset_;
		double amount_;
	};

	class Strategy
	{
	public:
		Strategy(const std::string& name);
		~Strategy();
		void Serialize(JsonSerializer* serializer);
		bool Parse(JsonDoc* json);
	public:
		// getter
		std::string name() const { return name_; }
		std::string description() const { return description_; }
		std::string version() const { return version_; }
		std::string author() const { return author_; }
		double init_cash() const { return init_cash_; }
		int num_portfolios() const { return num_portfolios_; }
		int step() const { return step_; }
		DATA_TYPE data_type() const { return data_type_; }
		int data_num() const { return data_num_; }
		time_t begin_time() const { return begin_time_; }
		time_t end_time() const { return end_time_; }
		std::string stocks() const { return stocks_; }
		bool sell_short() const { return sell_short_; }
		std::shared_ptr<IRule> screen_rule() const { return screen_rule_; }
		// setter
		void set_name(const std::string& name) { name_ = name; }
		void set_description(const std::string& description) { description_ = description; }
		void set_version(const std::string& version) { version_ = version; }
		void set_author(const std::string& author) { author_ = author; }
		void set_init_cash(double init_cash) { init_cash_ = init_cash; }
		void set_num_portfolios(int num_portfolios) { num_portfolios_ = num_portfolios; }
		void set_step(int step) { step_ = step; }
		void set_data_type(DATA_TYPE data_type) { data_type_ = data_type; } 
		void set_data_num(int data_num) { data_num_ = data_num; }
		void set_begin_time(time_t begin_time) { begin_time_ = begin_time; }
		void set_end_time(time_t end_time) { end_time_ = end_time; }
		void set_stocks(const std::string& stocks) { stocks_ = stocks; }
		void set_sell_short(bool sell_short) { sell_short_ = sell_short; }
		void set_screen_rule(std::shared_ptr<IRule> screen_rule) { screen_rule_ = screen_rule; }
	private:
		std::string name_;
		std::string description_;
		std::string version_;
		std::string author_;
		double init_cash_;
		int num_portfolios_;
		int step_;
		DATA_TYPE data_type_;
		int data_num_;
		time_t begin_time_;
		time_t end_time_;
		std::string stocks_;
		bool sell_short_;
		std::shared_ptr<IRule> screen_rule_;
	};

	// the environment for strategy to run
	class StrategyRunner
	{
	public:
		StrategyRunner(std::shared_ptr<IStore> store);
		~StrategyRunner();
	public:
		bool Load(const std::string& strategy_file);
		bool Initialize(bool reload = true);		
		void Run();		
		double GetBalance();
		void CleanUp();	
	public:
		Strategy* strategy() { return strategy_; }
		void set_observer(StrategyObserver* observer) { observer_ = observer; }
	private:
		void ScreenBuys();
		bool InBuysCandidates(Asset* asset);	
		void Trade();		
		void Buy();
		void Step();		
		void Sell() { Sell(portfolios_); }
		void Sell(std::vector<PortfolioItem>& portfolios);
		bool SellPortfolioItem(const PortfolioItem& item);
	private:
		std::shared_ptr<IStore> store_;
		AssetsProvider* assets_provider_;
		Strategy* strategy_;
		StrategyObserver* observer_;
		time_t cursor_;
		double cash_;
		AssetIndexerMap init_candidates_;
		AssetIndexerPtrMap buy_candidates_;

		std::vector<PortfolioItem> portfolios_;
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