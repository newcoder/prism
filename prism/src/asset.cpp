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

	AssetScale::AssetScale(const std::shared_ptr<Asset>& asset) : asset_(asset)
	{
		raw_data_ = nullptr;
		data_type_ = DATA_TYPE::DATA_TYPE_DAILY;
		data_num_ = 1;
	}

	AssetScale::~AssetScale()
	{
		// clear HLOC raw data and its indicators.
		ClearIndicators();
	}

	void AssetScale::ClearIndicators()
	{
		indicators_.clear();
	}

	std::string AssetScale::ToString(DATA_TYPE data_type, int data_num)
	{
		return std::to_string((int)data_type) + '_' + std::to_string(data_num);
	}

	void AssetScale::Create(DATA_TYPE data_type, int data_num)
	{
		std::shared_ptr<HLOCList> base_data = asset_->raw_data();
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
			raw_data_ = std::make_shared<HLOCList>();
			hs.ShrinkByWeek(raw_data_.get());
		}
		if (data_type == DATA_TYPE_MONTHLY)
		{
			HLOCSeries hs(base_data->begin(), base_data->end());
			raw_data_ = std::make_shared<HLOCList>();
			hs.ShrinkByMonth(raw_data_.get());
		}
		if (data_num > 1)
		{
			if (data_type_ == DATA_TYPE_DAILY)
				raw_data_ = base_data;
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			auto shrinked_data = std::make_shared<HLOCList>();
			hs.ShrinkByNum(data_num, shrinked_data.get());
			raw_data_ = shrinked_data;
		}
	}

	ILocalIndicator* AssetScale::indicators(const std::string& indicator_str)
	{
		auto it = indicators_.find(indicator_str);
		return it == indicators_.end() ? GenerateIndicator(indicator_str): it->second.get();
	}

	ILocalIndicator* AssetScale::GenerateIndicator(const std::string& indicator_str)
	{
		std::vector<std::string> elems;
		kyotocabinet::strsplit(indicator_str, '_', &elems);
		ILocalIndicator* indicator = nullptr;
		// MACD indicators
		if (elems[0] == "MACD")
		{
			int short_period = atoi(elems[1].c_str());
			int long_period = atoi(elems[2].c_str());
			int signal_period = atoi(elems[3].c_str());
			auto tl = std::make_unique<DoubleTimeList>();
			TLUtils::Populate(raw_data_.get(), PRICE_TYPE_C, tl.get());
			std::unique_ptr<ILocalIndicator> macd = std::make_unique<MACD>(short_period, long_period, signal_period);
			TimeSeries ts(tl->begin(), tl->end());
			ts.CalculateIndicator(macd.get());
			indicator = macd.get();
			indicators_.insert(std::make_pair(indicator_str, std::move(macd)));
		}

		// EMA indicators
		if (elems[0] == "EMA")
		{
			int period = atoi(elems[1].c_str());
			auto tl = std::make_unique<DoubleTimeList>();
			TLUtils::Populate(raw_data_.get(), PRICE_TYPE_C, tl.get());
			std::unique_ptr<ILocalIndicator> ema = std::make_unique<EMA>(period);
			TimeSeries ts(tl->begin(), tl->end());
			ts.CalculateIndicator(ema.get());
			indicator = ema.get();
			indicators_.insert(std::make_pair(indicator_str, std::move(ema)));
		}

		// CR indicators
		if (elems[0] == "CR")
		{
			int period = atoi(elems[1].c_str());
			std::unique_ptr<ILocalIndicator> cr = std::make_unique<CR>(period);
			HLOCSeries hs(raw_data_->begin(), raw_data_->end());
			hs.CalculateIndicator(cr.get());
			indicator = cr.get();
			indicators_.insert(std::make_pair(indicator_str, std::move(cr)));
		}

		return indicator;
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
		scales_.clear();
	}

	bool Asset::Load(IStore* store, size_t begin_year, size_t end_year)
	{
		raw_data_ = std::make_shared<HLOCList>();
		if (!store->Get(symbol_, begin_year, end_year, raw_data_.get()) || raw_data_->empty())
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

	std::shared_ptr<AssetScale> Asset::CreateScale(DATA_TYPE data_type, int data_num)
	{
		auto scale = std::make_shared<AssetScale>(shared_from_this());
		scale->Create(data_type, data_num);
		scales_.insert(std::make_pair(scale->to_string(), scale));
		return scale;
	}
	
	std::shared_ptr<AssetScale> Asset::scales(DATA_TYPE data_type, int data_num)
	{
		std::string scale_str = AssetScale::ToString(data_type, data_num);
		auto it = scales_.find(scale_str);
		return it == scales_.end() ? CreateScale(data_type, data_num) : it->second;
	}

	AssetsProvider::AssetsProvider(const std::shared_ptr<IStore>& store) : store_(store)
	{
	}
	
	AssetsProvider::~AssetsProvider()
	{
		Clear();				
	}

	void AssetsProvider::Clear()
	{
		assets_.clear();
	}

	int AssetsProvider::LoadAssets(const std::vector<std::string>& symbols, size_t begin_year, size_t end_year)
	{
		int count = 0;
		auto cit = symbols.begin();
		while(cit != symbols.end())
		{
			std::string symbol = *cit;
			auto asset = Get(symbol);
			if (asset == nullptr)
			{
				// not loaded
				asset = std::make_shared<Asset>(*cit);
				if (asset->Load(store_.get(), begin_year, end_year))
				{
					assets_.insert(std::make_pair(symbol, asset));
					count++;
				}
			}
			else
			{
				asset->Update(store_.get(), begin_year, end_year);
			}
			cit++;
		}
		return count;
	}

	std::shared_ptr<Asset> AssetsProvider::Get(const std::string& asset_string) const
	{
		auto cit = assets_.find(asset_string);
		return cit == assets_.end()? nullptr : cit->second;
	}

	int AssetsProvider::LoadAll()
	{
		std::vector<std::string> elems;
		std::string symbols_str;
		std::set<std::string> symbols_set;
		if (!store_->GetBlock(kBlockAllAShares, symbols_str))
			return -1;
		kyotocabinet::strsplit(symbols_str, '\n', &elems);

		// load symbols
		return LoadAssets(elems, 1992, GetYear(time(NULL)));
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


	AssetScaleIndexer::AssetScaleIndexer(const std::shared_ptr<AssetScale>& scale, time_t begin) : scale_(scale)
	{
		index_ = -1;
		ForwardTo(begin);
	}

	AssetScaleIndexer::AssetScaleIndexer(const std::shared_ptr<AssetScale>& scale) : scale_(scale)
	{
		index_ = -1;
	}

	void AssetScaleIndexer::ForwardTo(time_t pos)
	{
		if (pos < 0) return;
		auto hloc_list = scale_->raw_data();
		size_t i = index_ > 0 ? index_ : 0;
		while (i < hloc_list->size())
		{
			time_t time = hloc_list->at(i).time;
			if (time > pos) break;
			index_ = i;
			i++;
		}
	}

	time_t AssetScaleIndexer::GetIndexTime() const
	{
		if (index_ < 0)
			return -1;
		return scale_->raw_data()->at(index_).time;
	}

	bool AssetScaleIndexer::GetIndexData(HLOC& data) const
	{
		if (!valid())
			return false;
		data = scale_->raw_data()->at(index_);
		return true;
	}

	bool AssetScaleIndexer::within_limit() const
	{
		if (!valid())
			return false;
		if (index_ < 1)
			return true;
		auto data = scale_->raw_data()->at(index_);
		auto data_previous = scale_->raw_data()->at(index_ - 1);
		return std::fabs(data.open - data_previous.close) < kPriceLimit;
	}

	AssetIndexer::AssetIndexer(const std::shared_ptr<Asset>& asset, time_t begin) : AssetIndexer(asset)
	{
		ForwardTo(begin);
	}

	AssetIndexer::AssetIndexer(const std::shared_ptr<Asset>& asset) : asset_(asset), base_scale_indexer_(nullptr)
	{
		base_scale_indexer_ = scale_indexers(asset_->base_scale());
		assert(base_scale_indexer_);
	}

	void AssetIndexer::ForwardTo(time_t pos)
	{
		for (auto it : scale_indexers_)
			it.second->ForwardTo(pos);
	}

	void AssetIndexer::Forward()
	{
		for (auto it : scale_indexers_)
			it.second->Forward();
	}
	
	void AssetIndexer::Backward()
	{
		for (auto it : scale_indexers_)
			it.second->Backward();
	}

	void AssetIndexer::ToBegin()
	{
		for (auto it : scale_indexers_)
			it.second->ToBegin();
	}

	void AssetIndexer::ToEnd()
	{
		for (auto it : scale_indexers_)
			it.second->ToEnd();
	}

	std::shared_ptr<AssetScaleIndexer> AssetIndexer::scale_indexers(DATA_TYPE data_type, int data_num)
	{
		std::shared_ptr<AssetScale> scale = asset_->scales(data_type, data_num);
		return scale ? scale_indexers(scale) : nullptr;
	}

	std::shared_ptr<AssetScaleIndexer> AssetIndexer::scale_indexers(const std::shared_ptr<AssetScale>& scale)
	{
		auto cit = scale_indexers_.find(scale.get());
		if (cit != scale_indexers_.end())
			return cit->second;
		// align the initial index with the base scale
		time_t init_time = base_scale_indexer_ ? GetIndexTime() : -1;
		auto scale_indexer = std::make_shared<AssetScaleIndexer>(scale, init_time);
		scale_indexers_.insert(std::make_pair(scale.get(), scale_indexer));
		return scale_indexer;
	}


}