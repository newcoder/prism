// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// for asset and asset loader implementation.

#include "asset.h"
#include "store.h"
#include "indicator.h"
#include "time_series.h"
#include "hloc_series.h"
#include <iostream>

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
		for (std::map<std::string, ILocalIndicator*>::iterator it = indicators_.begin(); it != indicators_.end(); it++)
			delete it->second;
		//std::cout << "Asset::~Asset()" << std::endl;

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
		std::map<std::string, ILocalIndicator*>::iterator it = indicators_.find(indicator_str);
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

	AssetsLoader::AssetsLoader(IStore* store): store_(store)
	{
		//std::cout << "AssetsLoader::AssetsLoader()" << std::endl;
	}
	
	AssetsLoader::~AssetsLoader()
	{
		Clear();
				
		//std::cout << "AssetsLoader::~AssetsLoader()" << std::endl;
	}

	void AssetsLoader::Clear()
	{
		for (std::map<std::string, Asset*>::iterator it = assets_.begin(); it != assets_.end(); it++)
			delete it->second;	
		assets_.clear();
	}

	int AssetsLoader::LoadAssets(const std::vector<std::string>& symbols, size_t begin, size_t end, DATA_TYPE data_type, int data_num)
	{
		int count = 0;
		Clear();
		std::vector<std::string>::const_iterator cit = symbols.begin();
		while(cit != symbols.end())
		{
			Asset* asset = new Asset(*cit);
			if (asset->Load(store_, begin, end, data_type, data_num))
			{
				assets_.insert(std::make_pair(*cit, asset));
				count++;
			}
			cit++;
		}
		return count;
	}

	Asset* AssetsLoader::Get(const std::string& symbol)
	{
		std::map<std::string, Asset*>::const_iterator cit = assets_.find(symbol);
		return cit == assets_.end()? NULL : cit->second;
	}

}