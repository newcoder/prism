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

	AssetScale::AssetScale(Asset* asset) : asset_(asset)
	{
		raw_data_ = nullptr;
		data_type_ = DATA_TYPE::DATA_TYPE_DAILY;
		data_num_ = 1;
	}

	AssetScale::~AssetScale()
	{
		// clear HLOC raw data and its indicators.
		delete raw_data_;
		ClearIndicators();
	}

	void AssetScale::ClearIndicators()
	{
		for (auto it : indicators_)
			delete it.second;
		indicators_.clear();
	}

	std::string AssetScale::ToString(DATA_TYPE data_type, int data_num)
	{
		return std::to_string((int)data_type) + '_' + std::to_string(data_num);
	}

	void AssetScale::Create(DATA_TYPE data_type, int data_num)
	{
		HLOCList* base_data = asset_->raw_data();
		assert(base_data);
		data_type_ = data_type;
		data_num_ = data_num;

		if (data_type_ == DATA_TYPE_DAILY && data_num_ == 1)
		{
			// the base scale, use the raw data in Asset
			raw_data_ = base_data;
			return;
		}
		if (data_type == DATA_TYPE_WEEKLY)
		{
			HLOCSeries hs(base_data->begin(), base_data->end());
			raw_data_ = new HLOCList();
			hs.ShrinkByWeek(raw_data_);
		}
		if (data_type == DATA_TYPE_MONTHLY)
		{
			HLOCSeries hs(base_data->begin(), base_data->end());
			raw_data_ = new HLOCList();
			hs.ShrinkByMonth(raw_data_);
		}
		if (data_num > 1)
		{
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			HLOCList* shrinked_data = new HLOCList();
			hs.ShrinkByNum(data_num, shrinked_data);
			delete raw_data_;
			raw_data_ = shrinked_data;
		}
	}

	ILocalIndicator* AssetScale::indicators(const std::string& indicator_str)
	{
		auto it = indicators_.find(indicator_str);
		return it == indicators_.end() ? GenerateIndicator(indicator_str): it->second;
	}

	ILocalIndicator* AssetScale::GenerateIndicator(const std::string& indicator_str)
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

	Asset::Asset(const std::string& symbol) : symbol_(symbol)
	{
	}

	Asset::~Asset()
	{
		ClearScales();
	}

	void Asset::ClearScales()
	{
		for (auto it : scales_)
			delete it.second;
		scales_.clear();
	}

	bool Asset::Load(IStore* store, size_t begin_year, size_t end_year)
	{
		raw_data_ = new HLOCList();
		if (!store->Get(symbol_, begin_year, end_year, raw_data_))
			return false;
		begin_year_ = begin_year;
		end_year_ = end_year;
		// create the base scale
		CreateScale(DATA_TYPE_DAILY, 1);
		return true;
	}

	bool Asset::Update(IStore* store, size_t begin_year, size_t end_year)
	{
		if (begin_year >= begin_year_ && end_year <= end_year_)
		{
			return true;
		}
		ClearScales();
		return Load(store, begin_year, end_year);
	}

	AssetScale* Asset::CreateScale(DATA_TYPE data_type, int data_num)
	{
		AssetScale* scale = new AssetScale(this);
		scale->Create(data_type, data_num);
		scales_.insert(std::make_pair(scale->to_string(), scale));
		return scale;
	}
	
	AssetScale* Asset::scales(DATA_TYPE data_type, int data_num)
	{
		std::string scale_str = AssetScale::ToString(data_type, data_num);
		auto it = scales_.find(scale_str);
		return it == scales_.end() ? CreateScale(data_type, data_num) : it->second;
	}

	AssetsProvider::AssetsProvider(IStore* store) : store_(store)
	{
	}
	
	AssetsProvider::~AssetsProvider()
	{
		Clear();				
	}

	void AssetsProvider::Clear()
	{
		for (auto it : assets_)
			delete it.second;	
		assets_.clear();
	}

	int AssetsProvider::LoadAssets(const std::vector<std::string>& symbols, size_t begin_year, size_t end_year)
	{
		int count = 0;
		auto cit = symbols.begin();
		while(cit != symbols.end())
		{
			std::string symbol = *cit;
			Asset* asset = Get(symbol);
			if (asset == NULL)
			{
				// not loaded
				asset = new Asset(*cit);
				if (asset->Load(store_, begin_year, end_year))
				{
					assets_.insert(std::make_pair(symbol, asset));
					count++;
				}
			}
			else
			{
				asset->Update(store_, begin_year, end_year);
			}
			cit++;
		}
		return count;
	}

	Asset* AssetsProvider::Get(const std::string& asset_string) const
	{
		auto cit = assets_.find(asset_string);
		return cit == assets_.end()? NULL : cit->second;
	}

	bool AssetsProvider::LoadAll()
	{
		std::vector<std::string> elems;
		std::string symbols_str;
		std::set<std::string> symbols_set;
		if (!store_->GetBlock(kBlockAllAShares, symbols_str))
			return false;
		kyotocabinet::strsplit(symbols_str, '\n', &elems);

		// load symbols
		LoadAssets(elems, 1992, GetYear(time(NULL)));
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

	int AssetsProvider::LoadAssets(const std::string& symbols_pattern, size_t begin_year, size_t end_year)
	{
		std::vector<std::string> symbols;
		if (!ParseSymbolsPattern(symbols_pattern, symbols))
			return -1;
		// load symbols
		return LoadAssets(symbols, begin_year, end_year);
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