// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// for strategy implementation.

#include "strategy.h"
#include "rule.h"
#include "store.h"
#include "asset.h"
#include <ctime>
#include <assert.h>
#include <set>
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"

// kc utils
#include <kcutil.h>

namespace prism {
	
	Strategy::Strategy(const std::string& name): name_(name)
	{
		screen_rule_ = NULL;
		//std::cout << "Strategy::Strategy()" << std::endl;
	}

	Strategy::~Strategy()
	{
		delete screen_rule_;
		//std::cout << "Strategy::~Strategy()" << std::endl;

	}
	
	/*
	"description": "sample for strategy",
	"author": "rayni",    
	"version": "0.1",
	"sell_short": false,
	"begin_time": "1992-01-01",
	"end_time": "2012-12-31",
	"step": 1,
	"init_cash": 1000000,
	"stocks": "SH600\\d{3}",
	"screen_rule": {}
	**/
	void Strategy::Serialize(JsonSerializer* serializer)
	{
		serializer->StartObject();
		serializer->String("description");
		serializer->String(description_.c_str());
		serializer->String("author");
		serializer->String(author_.c_str());
		serializer->String("version");
		serializer->String(version_.c_str());
		serializer->String("sell_short");
		serializer->Bool(sell_short_);		
		std::string begin = TimeToString(begin_time_, "%Y-%m-%d");
		serializer->String("begin_time");
		serializer->String(begin.c_str());
		std::string end = TimeToString(end_time_, "%Y-%m-%d");
		serializer->String("end_time");
		serializer->String(end.c_str());
		serializer->String("step");
		serializer->Int(step_);	
		serializer->String("data_type");
		serializer->Int((int)data_type_);
		serializer->String("data_num");
		serializer->Int(data_num_);
		serializer->String("init_cash");
		serializer->Double(init_cash_);
		serializer->String("num_portfolios");
		serializer->Int(num_portfolios_);
		serializer->String("stocks");
		serializer->String(stocks_.c_str());
		// the screen rule
		serializer->String("screen_rule");
		serializer->StartObject();
		screen_rule_->Serialize(serializer);
		serializer->EndObject();

		serializer->EndObject();
	}

	bool Strategy::Parse(JsonDoc* json)
	{
		description_ = json->operator[]("description").GetString();
		version_ = json->operator[]("version").GetString();
		author_ = json->operator[]("author").GetString();
		init_cash_ = json->operator[]("init_cash").GetDouble();
		num_portfolios_ = json->operator[]("num_portfolios").GetInt();
		step_ = json->operator[]("step").GetInt();
		data_type_ = (DATA_TYPE)json->operator[]("data_type").GetInt();
		data_num_ = json->operator[]("data_num").GetInt();
		std::string begin = json->operator[]("begin_time").GetString();
		begin_time_ = StringToDate(begin, "%d-%d-%d");
		std::string end = json->operator[]("end_time").GetString();
		end_time_ = StringToDate(end, "%d-%d-%d");
		stocks_ = json->operator[]("stocks").GetString();
		sell_short_ = json->operator[]("sell_short").GetBool();

		JsonValue& jsonRule = json->operator[]("screen_rule");
		assert(jsonRule.IsObject());
		screen_rule_ = RuleFactory::CreateRule(&jsonRule);
		return true;
	}

	AssetsProvider::AssetsProvider(IStore* store) : store_(store), loader_(nullptr)
	{
	}

	AssetsProvider::~AssetsProvider()
	{
		delete loader_;
	}

	bool AssetsProvider::LoadAll(DATA_TYPE type)
	{
		std::vector<std::string> elems;
		std::string symbols_str;
		std::set<std::string> symbols_set;
		if (!store_->GetBlock(kBlockAllAShares, symbols_str))
			return false;
		kyotocabinet::strsplit(symbols_str, '\n', &elems);
		
		if (loader_ != NULL)
			delete loader_;
		loader_ = new AssetsLoader(store_);

		// load symbols
		loader_->LoadAssets(elems, 1992, 2013, type, 1);
		return true;
	}

	bool AssetsProvider::LoadForStrategy(Strategy* strategy)
	{
		std::vector<std::string> elems, blocks, patterns;
		std::string stocks = strategy->stocks();
		kyotocabinet::strsplit(stocks, ';', &elems);
		for (size_t i = 0; i < elems.size(); ++i)
		{
			if (elems[i].find(kStockBlocks) != std::string::npos)
			{
				std::string block_list = elems[i].substr(elems[i].find("=") + 1);
				kyotocabinet::strsplit(block_list, ',', &blocks);
			}

			if (elems[i].find(kStockPatterns) != std::string::npos)
			{
				std::string pattern_list = elems[i].substr(elems[i].find("=") + 1);
				kyotocabinet::strsplit(pattern_list, ',', &patterns);
			}
		}

		// load symbols list from blocks		
		std::string symbols_str;
		std::set<std::string> symbols_set;
		if (blocks.empty())
		{
			blocks.push_back(kBlockAllAShares);
		}
		for (size_t i = 0; i < blocks.size(); ++i)
		{
			if (blocks[i].empty()) continue;
			if (!store_->GetBlock(blocks[i], symbols_str))
				return false;
			kyotocabinet::strsplit(symbols_str, '\n', &elems);
			for (size_t j = 0; j < elems.size(); ++j)
			{
				if (!elems[j].empty())
					symbols_set.insert(elems[j]);
			}
		}

		// filter symbols by the patterns
		std::vector<std::string> symbols_filtered;
		std::set<std::string>::const_iterator cit = symbols_set.begin();
		while (cit != symbols_set.end())
		{
			for (size_t j = 0; j < patterns.size(); ++j)
			{
				boost::regex pattern(patterns[j]);
				if (regex_match(*cit, pattern))
				{
					symbols_filtered.push_back(*cit);
				}
			}
			cit++;
		}

		if (loader_ != NULL)
			delete loader_;
		loader_ = new AssetsLoader(store_);

		// load symbols
		loader_->LoadAssets(symbols_filtered,
			GetYear(strategy->begin_time()),
			GetYear(strategy->end_time()),
			strategy->data_type(),
			strategy->data_num());

		return true;
	}


	AssetIndexer::AssetIndexer(Asset* asset, time_t begin) : asset_(asset)
	{
		index_ = -1;
		MoveTo(begin);
	}
	
	AssetIndexer::AssetIndexer(Asset* asset) : asset_(asset)
	{
		index_ = -1;
	}

	void AssetIndexer::MoveTo(time_t pos)
	{
		HLOCList* hloc_list = asset_->raw_data();
		size_t i = index_ > 0? index_ : 0;
		while (i < hloc_list->size())
		{
			time_t time = hloc_list->at(i).time;
			if (time <= pos)
			{
				index_ = i;
				i++;
			} else 
				{
					break;
			}
		}
	}
	
	bool AssetIndexer::at_end()
	{
		return index_ == asset_->raw_data()->size() - 1;
	}

	time_t AssetIndexer::GetIndexTime()
	{
		if (index_ < 0)
			return -1;
		return asset_->raw_data()->at(index_).time;
	}

	StrategyRunner::StrategyRunner(IStore* store)
	{
		store_ = store;
		strategy_ = NULL;
		assets_provider_ = new AssetsProvider(store);
		//std::cout << "StrategyRunner::StrategyRunner()" << std::endl;
	}

	StrategyRunner::~StrategyRunner()
	{
		delete strategy_;
		delete assets_provider_;
		//std::cout << "StrategyRunner::~StrategyRunner()" << std::endl;
	}

	bool StrategyRunner::Initialize(bool reload)
	{
		assert(store_);
		assert(strategy_); 
		cash_ = strategy_->init_cash();
		// re-load assets
		if (reload)
		{
			if (!assets_provider_->LoadForStrategy(strategy_))
				return false;
		}
		cursor_ = strategy_->begin_time();
		init_candidates_.clear();
		std::map<std::string, Asset*> *loaded_assets = assets_provider_->loader()->assets();
		std::map<std::string, Asset*>::iterator it = loaded_assets->begin();
		while(it != loaded_assets->end())
		{
			init_candidates_.insert(std::make_pair(it->second->symbol(),AssetIndexer(it->second, cursor_)));
			it++;
		}
		return true;
	}

	void StrategyRunner::ScreenBuys()
	{
		// filter the buys candidates by the screen rule in strategy
		AssetIndexerMap::iterator it = init_candidates_.begin();
		buy_candidates_.clear();
		while (it != init_candidates_.end())
		{
			IRule* rule = strategy_->screen_rule();
			AssetIndexer *asset_indexer = &(it->second);
			time_t time = asset_indexer->GetIndexTime();
			if (time > 0)
			{
				if (!asset_indexer->at_end() && rule->Verify(asset_indexer->asset(), asset_indexer->index()))
				{
					buy_candidates_.insert(std::make_pair(asset_indexer->asset()->symbol(), asset_indexer));
				}
			}
			it++;
		}
	}

	bool StrategyRunner::InBuysCandidates(Asset* asset)
	{
		AssetIndexerPtrMap::const_iterator cit = buy_candidates_.find(asset->symbol());
		return cit != buy_candidates_.end();
	}

	void StrategyRunner::Run()
	{
		if (observer_)
			observer_->OnStart();

		while (cursor_ < strategy_->end_time())
		{
			if (observer_)
				observer_->OnCycleBegin(cursor_);
			ScreenBuys();
			Step();			
			Trade();
			if (observer_)
				observer_->OnCycleEnd(cursor_);
		}

		if (observer_)
			observer_->OnFinished();
	}

	void StrategyRunner::Step()
	{
		cursor_ = cursor_ + 3600 * 24 * strategy_->step();
		AssetIndexerMap::iterator it = init_candidates_.begin();
		while (it != init_candidates_.end())
		{
			it->second.MoveTo(cursor_);
			//std::cout << "after MoveTo, index: " << it->second.index() << TimeToString(cursor_, ", cursor: %Y-%m-%d") << std::endl;
			it++;
		}
	}

	void StrategyRunner::Trade()
	{
		// sell portfolio if it is not in buy candidates
		Sell();
		// buy stocks if available
		Buy();
	}

	void StrategyRunner::Sell(std::vector<PortfolioItem>& portfolios)
	{		
		bool sold = false;
		std::vector<PortfolioItem>::iterator it = portfolios.begin();
		while (it != portfolios.end())
		{
			if (!InBuysCandidates((*it).asset()))
			{
				sold = SellPortfolioItem(*it);
				if (sold)
				{
					portfolios.erase(it);
					break;
				}
			}
			it++;
		}
		if (sold)
			Sell(portfolios);
	}

	bool StrategyRunner::SellPortfolioItem(const PortfolioItem& item)
	{
		AssetIndexerMap::const_iterator cit = init_candidates_.find(item.asset()->symbol());
		assert(cit != init_candidates_.end());
		AssetIndexer asset_indexer = cit->second;
		assert(item.asset() == asset_indexer.asset());
		if (asset_indexer.GetIndexTime() == cursor_)
		{
			// the stock was on trade at the time
			size_t index = asset_indexer.index();
			double amount = item.amount();
			// assume the stock saled at open price
			double price = item.asset()->raw_data()->at(index).open;
			double money = price * amount;
			// add transaction log
			Transaction trans;
			trans.asset_ = item.asset();
			trans.price_ = price;
			trans.shares_ = item.amount();
			trans.time_ = asset_indexer.GetIndexTime();
			trans.type_ = TRANSACTION_TYPE_SELL;
			trans.commission_ = kCommissionRate * money;
			cash_ = cash_ + money - trans.commission_;		
			if (observer_)
				observer_->OnTransaction(trans);
			return true;
		}
		return false;
	}

	void StrategyRunner::Buy()
	{
		if (cash_ < kEpsilon)
			return;
		int num_can_buy = strategy_->num_portfolios() - portfolios_.size();
		int num_to_buy = num_can_buy < buy_candidates_.size() ? num_can_buy : buy_candidates_.size();
		if (num_to_buy <= 0)
			return;

		srand(time(NULL));		
		double money = cash_ / num_to_buy;			
		for (int i = 0; i < num_to_buy; ++i)
		{	
			size_t k = rand() % buy_candidates_.size();
			AssetIndexerPtrMap::const_iterator cit = buy_candidates_.begin();
			while (k > 0) { cit++; k--; }
			AssetIndexer *asset_indexer = cit->second;
			if (asset_indexer->GetIndexTime() == cursor_)
			{
				size_t index = asset_indexer->index();
				double price = asset_indexer->asset()->raw_data()->at(index).open;
				int amount_hands = (int)(money / (kHand * (1 + kCommissionRate)*price));
				if (amount_hands > 0)
				{
					double shares = 100*amount_hands;
					money = shares * price;
					Transaction trans;
					trans.asset_ = asset_indexer->asset();
					trans.price_ = price;
					trans.shares_ = shares;
					trans.time_ = asset_indexer->GetIndexTime();
					trans.type_ = TRANSACTION_TYPE_BUY;
					trans.commission_ = kCommissionRate * money;
					cash_ = cash_ - money - trans.commission_;
					portfolios_.push_back(PortfolioItem(trans.asset_, trans.shares_));
					if (observer_)
						observer_->OnTransaction(trans);
				}
			}
		}

	}

	void StrategyRunner::CleanUp()
	{		
		init_candidates_.clear();
		buy_candidates_.clear();
		portfolios_.clear();
	}	

	bool StrategyRunner::Load(const std::string& strategy_file)
	{
		prism::JsonDoc doc;
		bool ret = prism::ParseJson(strategy_file, doc);
		boost::filesystem::path strategy_path(strategy_file);
		strategy_ = new Strategy(strategy_path.filename().string());
		ret = strategy_->Parse(&doc);
		return ret;
	}

	double StrategyRunner::GetBalance()
	{
		std::vector<PortfolioItem>::const_iterator cit = portfolios_.begin();
		double balance = cash_;
		while (cit != portfolios_.end())
		{
			PortfolioItem item = *cit; 
			AssetIndexerMap::const_iterator cit_asset = init_candidates_.find(item.asset()->symbol());
			assert(cit_asset != init_candidates_.end());
			AssetIndexer asset_indexer = cit_asset->second;
			assert(item.asset() == indexed_asset.asset());
			double price = item.asset()->raw_data()->at(asset_indexer.index()).close;
			balance += price * item.amount() * (1 - kCommissionRate);
			cit++;
		}
		return balance;
	}

	StrategyObserver::StrategyObserver(StrategyRunner* runner): runner_(runner)
	{
		//std::cout << "StrategyObserver::StrategyObserver()" << std::endl;
	}

	StrategyObserver::~StrategyObserver()
	{
		//std::cout << "StrategyObserver::~StrategyObserver()" << std::endl;
	}

	void StrategyObserver::OnTransaction(const Transaction& trans)
	{
		transactions_.push_back(trans);
/*		std::cout << trans.type_ << ", " 
			<< trans.asset_->symbol() << ", " 
			<< TimeToString(trans.time_, "%Y-%m-%d, ") 
			<< trans.price_ << ", " 
			<< trans.shares_<< std::endl;	*/						
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
		p.value = runner_->GetBalance();
		performance_series_.push_back(p);
	}

	void StrategyObserver::OnFinished()
	{
		//std::cout << "transactions: "<< transactions_.size() << std::endl;
		//std::cout << "wins: "<< GetTransactionsNum(true) << std::endl;
		double balance = runner_->GetBalance();
		double init = runner_->strategy()->init_cash();
		profit_ = (balance - init) * 100 / init;
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
			std::string key = trans.asset_->symbol() + std::to_string(static_cast<long long>(amount));			
			if (trans.type_ == TRANSACTION_TYPE_BUY)
			{
				symbol_trans.insert(std::make_pair(key, trans));
			}
			else
			{
				std::map<std::string, Transaction>::iterator it = symbol_trans.find(key);
				//assert(it != symbol_trans.end());
				if (it != symbol_trans.end())
				{
					if ((it->second.price_ < trans.price_ && win) || (it->second.price_ > trans.price_ && !win))
						num++;
					symbol_trans.erase(it);
				}
			}
			cit++;
		}
		return num;
	}

}