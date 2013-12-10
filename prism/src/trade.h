// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// routines and classes for trade
// transaction, portfolio manager, etc..

#ifndef TRADE_H
#define TRADE_H

#include "common.h"
#include "util.h"
#include "time.h"
#include "asset.h"
#include <string>
#include <vector>
#include <map>

namespace prism {
	
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
		int transaction_id_;
		TRANSACTION_TYPE type_;
		AssetIndexer* asset_indexer_;
		double price_;
		double shares_;
		double commission_;
		time_t time_;
	};

	typedef std::vector<Transaction> TransactionList;

	class TransactionManager
	{
	public:
		TransactionManager(){}
		~TransactionManager(){}
	public:
		void Add(Transaction& trans);
		// get transactions for a single symbol
		void GetTransactions(const std::string& symbol, TransactionList* symbol_transactions);
	private:
		static unsigned int id;
		TransactionList transacitons_;
	};

	class Portfolio
	{
	public:
		Portfolio() : asset_indexer_(nullptr), amount_(0.0) {}
		Portfolio(AssetIndexer* asset_indexer, double amount): asset_indexer_(asset_indexer), amount_(amount) {}
	public:
		void Buy(double amount) { amount_ += amount; }
		void Sell(double amount) { amount_ -= amount; }
		bool GetValue(time_t time, double& value);		
		std::string symbol() { return asset_indexer_->asset()->symbol(); }
		AssetIndexer* asset_indexer() const { return asset_indexer_; }
		double amount() const { return amount_; }
		void set_asset(AssetIndexer* asset_indexer) { asset_indexer_ = asset_indexer; }
		void set_amount(double amount) { amount_ = amount; }
	private:
		AssetIndexer* asset_indexer_;
		double amount_;
	};

	class PortfolioManager
	{
	public:
		PortfolioManager() {}
		~PortfolioManager() {}
	public:
		void Buy(AssetIndexer* asset_indexer, double amount);
		void Sell(AssetIndexer* asset_indexer, double amount);
		void Clear();
	private:
		std::map<std::string, Portfolio*> portfolios_;
	};

}

#endif