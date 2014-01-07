// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
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
		std::weak_ptr<AssetIndexer> asset_indexer_;
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
		size_t Size() { return transactions_.size(); }
		void Clear() { transactions_.clear(); }
		// get transactions for a single symbol
		void GetTransactions(const std::string& symbol, TransactionList* symbol_transactions);
	public:
		static unsigned int id;
	private:	
		TransactionList transactions_;
	};

	class Portfolio
	{
	public:
		Portfolio(const std::weak_ptr<AssetIndexer>& asset_indexer, double amount): asset_indexer_(asset_indexer), amount_(amount) {}
	public:
		void Buy(double amount) { amount_ += amount; }
		void Sell(double amount) { amount_ -= amount; }
		bool GetValue(double& value, time_t pos);		
		std::string symbol();
		std::weak_ptr<AssetIndexer> asset_indexer() const { return asset_indexer_; }
		double amount() const { return amount_; }
		void set_asset(std::weak_ptr<AssetIndexer> asset_indexer) { asset_indexer_ = asset_indexer; }
		void set_amount(double amount) { amount_ = amount; }
	private:
		std::weak_ptr<AssetIndexer> asset_indexer_;
		double amount_;
	};

	class PortfolioManager
	{
	public:
		PortfolioManager() {}
		~PortfolioManager() {}
	public:
		void Buy(const std::weak_ptr<AssetIndexer>& asset_indexer, double amount);
		void Sell(const std::weak_ptr<AssetIndexer>& asset_indexer, double amount);
		// move to the pos if pos > 0, then evaluate the value
		bool GetValue(double& value, time_t pos = -1);
		void Clear();
		Portfolio* Get(const std::string& symbol);
	private:
		std::map<std::string, std::unique_ptr<Portfolio>> portfolios_;
	};

}

#endif