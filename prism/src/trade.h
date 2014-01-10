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
		int associate_transaction_id;
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
		size_t Size() { return transactions_.size(); }
		void Clear() { transactions_.clear(); }
		Transaction* Get(int id);
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
		Portfolio(const Transaction& trans): trans_(trans) {}
	public:
		bool GetValue(double& value, time_t pos = -1);		
		std::string symbol() { return trans_.asset_indexer_->asset()->symbol(); }
		int transaction_id() { return trans_.transaction_id_; }
		AssetIndexer* asset_indexer() const { return trans_.asset_indexer_; }
		double shares() const { return trans_.shares_; }
	private:
		Transaction trans_; // the transaction of buy..or sell short...
	};

	typedef std::vector<Portfolio> PortfolioList;

	class PortfolioManager
	{
	public:
		PortfolioManager() {}
		~PortfolioManager() {}
	public:
		void Remove(int transaction_id);
		void Add(const Portfolio& portfolio) { portfolios_.push_back(portfolio); }
		// move to the pos if pos > 0, then evaluate the value
		bool GetValue(double& value, time_t pos = -1);
		void Clear() { return portfolios_.clear(); }
		size_t Size() { return portfolios_.size(); }
		void GetPortfolios(const std::string& symbol, PortfolioList* portfolios);
	private:
		std::vector<Portfolio> portfolios_;
	};

	class CashBox
	{
	public:
		CashBox(double total, int num_slots) : total_(total), num_slots_(num_slots) { Init(); }
		~CashBox(){}
	public:
		void Init();
		// get cash from a slot
		bool Get(double &amount);
		// put cash to an empty slot
		void Put(double amount);
	private:
		double total_;
		int num_slots_;
		std::vector<double> slots_;
	};

}

#endif