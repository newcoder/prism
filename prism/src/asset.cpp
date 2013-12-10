// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// for asset and asset loader implementation.

#include "asset.h"
#include "store.h"
#include "indicator.h"
#include "time_series.h"
#include "hloc_series.h"
#include "util.h"
#include <iostream>
#include "boost/regex.hpp"

// kc utils
#include <kcutil.h>

namespace prism {

	Asset::Asset(const std::string& symbol): symbol_(symbol)
	{
		raw_data_ = new HLOCList();
		//std::cout << "Asset::Asset()" << std::endl;

	}

	Asset::~Asset()
	{
		// clear HLOC raw data and its indicators.
		delete raw_data_;
		//std::cout << "Asset::~Asset()" << std::endl;
		ClearIndicators();
	}

	std::string Asset::ToSymbolString(const std::string& symbol,
		size_t begin_year,
		size_t end_year,
		DATA_TYPE data_type,
		int data_num)
	{
		return symbol + '_' + std::to_string(data_type) + '_' + std::to_string(data_num) + '_' +
			std::to_string(begin_year) + '_' + std::to_string(end_year);
	}

	void Asset::ClearIndicators()
	{
		for (std::map<std::string, ILocalIndicator*>::iterator it = indicators_.begin(); it != indicators_.end(); it++)
			delete it->second;
		indicators_.clear();
	}

	bool Asset::Load(IStore* store, size_t begin_year, size_t end_year, DATA_TYPE data_type, int data_num)
	{
		if (!store->Get(symbol_, begin_year, end_year, raw_data_))
			return false;
		if (data_type == DATA_TYPE_WEEKLY)
		{
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			HLOCList* weekly_data = new HLOCList();
			hs.ShrinkByWeek(weekly_data);
			delete raw_data_;
			raw_data_ = weekly_data;
		}
		if (data_type == DATA_TYPE_MONTHLY)
		{
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			HLOCList* monthly_data = new HLOCList();
			hs.ShrinkByWeek(monthly_data);
			delete raw_data_;
			raw_data_ = monthly_data;
		}
		if (data_num > 1)
		{
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			HLOCList* shrinked_data = new HLOCList();
			hs.ShrinkByNum(data_num, shrinked_data);
			delete raw_data_;
			raw_data_ = shrinked_data;
		}
		data_type_ = data_type;
		data_num_ = data_num;
		begin_year_ = begin_year;
		end_year_ = end_year;
		return true;
	}

	ILocalIndicator* Asset::indicators(const std::string& indicator_str)
	{
		auto it = indicators_.find(indicator_str);
		return it == indicators_.end() ? GenerateIndicator(indicator_str): it->second;
	}

	ILocalIndicator* Asset::GenerateIndicator(const std::string& indicator_str)
	{
		std::vector<std::string> elems;
		kyotocabinet::strsplit(indicator_str, '_', &elems);
		// MACD indicators
		if (elems[0] == "MACD")
		{
			int short_period = atoi(elems[1].c_str());
			int long_period = atoi(elems[2].c_str());
			int signal_period = atoi(elems[3].c_str());
			DoubleTimeList tl;
			TLUtils::Populate(raw_data_, PRICE_TYPE_C, &tl);
			MACD* macd = new MACD(short_period, long_period, signal_period);
			TimeSeries ts(tl.begin(), tl.end());
			ts.CalculateIndicator(macd);
			indicators_.insert(std::make_pair(indicator_str, macd));
			return macd;
		}

		// EMA indicators
		if (elems[0] == "EMA")
		{
			int period = atoi(elems[1].c_str());
			DoubleTimeList tl;
			TLUtils::Populate(raw_data_, PRICE_TYPE_C, &tl);
			EMA* ema = new EMA(period);
			TimeSeries ts(tl.begin(), tl.end());
			ts.CalculateIndicator(ema);
			indicators_.insert(std::make_pair(indicator_str, ema));
			return ema;
		}

		// CR indicators
		if (elems[0] == "CR")
		{
			int period = atoi(elems[1].c_str());
			DoubleTimeList tl;
			CR* cr = new CR(period);
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			hs.CalculateIndicator(cr);
			indicators_.insert(std::make_pair(indicator_str, cr));
			return cr;
		}

		return NULL;
	}

	AssetsProvider::AssetsProvider(IStore* store) : store_(store)
	{
		//std::cout << "AssetsLoader::AssetsLoader()" << std::endl;
	}
	
	AssetsProvider::~AssetsProvider()
	{
		Clear();
				
		//std::cout << "AssetsLoader::~AssetsLoader()" << std::endl;
	}

	void AssetsProvider::Clear()
	{
		for (auto it : assets_)
			delete it.second;	
		assets_.clear();
	}

	int AssetsProvider::LoadAssets(const std::vector<std::string>& symbols,
		size_t begin_year, 
		size_t end_year, 
		DATA_TYPE data_type, 
		int data_num)
	{
		int count = 0;
		auto cit = symbols.begin();
		while(cit != symbols.end())
		{
			std::string symbol_string = Asset::ToSymbolString(*cit, begin_year, end_year, data_type, data_num);
			Asset* asset = Get(symbol_string);
			if (asset == NULL)
			{
				// not loaded
				asset = new Asset(*cit);
				if (asset->Load(store_, begin_year, end_year, data_type, data_num))
				{
					assets_.insert(std::make_pair(symbol_string, asset));
					count++;
				}
			}
			cit++;
		}
		return count;
	}

	Asset* AssetsProvider::Get(const std::string& symbol_string) const
	{
		auto cit = assets_.find(symbol_string);
		return cit == assets_.end()? NULL : cit->second;
	}

	bool AssetsProvider::LoadAll(DATA_TYPE type)
	{
		std::vector<std::string> elems;
		std::string symbols_str;
		std::set<std::string> symbols_set;
		if (!store_->GetBlock(kBlockAllAShares, symbols_str))
			return false;
		kyotocabinet::strsplit(symbols_str, '\n', &elems);

		// load symbols
		LoadAssets(elems, 1992, 2013, type, 1);
		return true;
	}

	bool AssetsProvider::ParseSymbolsPattern(const std::string& symbols_pattern, std::vector<std::string>& symbols)
	{
		std::vector<std::string> elems, blocks, patterns;
		std::string stocks = symbols_pattern;
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
		auto cit = symbols_set.begin();
		while (cit != symbols_set.end())
		{
			for (size_t j = 0; j < patterns.size(); ++j)
			{
				boost::regex pattern(patterns[j]);
				if (regex_match(*cit, pattern))
				{
					symbols.push_back(*cit);
				}
			}
			cit++;
		}

		return true;
	}

	int AssetsProvider::LoadAssets(const std::string& symbols_pattern,
		size_t begin_year,
		size_t end_year,
		DATA_TYPE data_type,
		int data_num)
	{
		std::vector<std::string> symbols;
		if (!ParseSymbolsPattern(symbols_pattern, symbols))
			return -1;
		// load symbols
		return LoadAssets(symbols, begin_year, end_year, data_type, data_num);
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
		size_t i = index_ > 0 ? index_ : 0;
		while (i < hloc_list->size())
		{
			time_t time = hloc_list->at(i).time;
			if (time <= pos)
			{
				index_ = i;
				i++;
			}
			else
			{
				break;
			}
		}
	}

	time_t AssetIndexer::GetIndexTime() const
	{
		if (index_ < 0)
			return -1;
		return asset_->raw_data()->at(index_).time;
	}

	bool AssetIndexer::GetIndexData(HLOC* data) const
	{
		if (!valid())
			return false;
		data = &(asset_->raw_data()->at(index_));
		return true;
	}

}