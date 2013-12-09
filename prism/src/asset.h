// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// routines and classes for asset, stock is a kind of asset..

#ifndef ASSET_H
#define ASSET_H

#include "common.h"
#include "time.h"
#include <string>
#include <vector>
#include <map>

namespace prism {

	class ILocalIndicator;
	class IStore;

	class Asset
	{
	public:
		Asset(const std::string& symbol);
		~Asset();
		bool Load(IStore* store, size_t begin_year, size_t end_year, DATA_TYPE data_type = DATA_TYPE_DAILY, int data_num = 1);
		void ClearIndicators();
		HLOCList* raw_data() { return raw_data_; }
		std::string symbol() { return symbol_; }
		DATA_TYPE data_type() { return data_type_; }
		int data_num() { return data_num_; }
		size_t begin_year() { return begin_year_; }
		size_t end_year() { return end_year_; }
		ILocalIndicator* indicators(const std::string& indicator_str);
	private:
		ILocalIndicator* GenerateIndicator(const std::string& indicator_str);
	public:
		static std::string ToSymbolString(const std::string& symbol,
			size_t begin_year,
			size_t end_year,
			DATA_TYPE data_type,
			int data_num);
	private:
		std::string symbol_;
		DATA_TYPE data_type_;
		int data_num_;
		size_t begin_year_;
		size_t end_year_;
		HLOCList* raw_data_;
		std::map<std::string, ILocalIndicator*> indicators_;
	};

	class AssetsProvider
	{
	public:
		AssetsProvider(IStore* store);
		~AssetsProvider();
		void Clear();
		int LoadAssets(const std::string& symbols_pattern, 
			size_t begin_year, 
			size_t end_year, 
			DATA_TYPE data_type = DATA_TYPE_DAILY, 
			int data_num = 1);
		int LoadAssets(const std::vector<std::string>& symbols, 
			size_t begin_year, 
			size_t end_year, 
			DATA_TYPE data_type = DATA_TYPE_DAILY, 
			int data_num = 1);
		bool LoadAll(DATA_TYPE type);
		Asset* Get(const std::string& symbol_string);
		std::map<std::string, Asset*>* assets() { return &assets_; }
	private:
		std::string ToSymbolString(const std::string& symbol, 
			size_t begin_year,
			size_t end_year,
			DATA_TYPE data_type,
			int data_num);
		bool ParseSymbolsPattern(const std::string& symbols_pattern, std::vector<std::string>& symbols);
	private:
		IStore* store_;
		std::map<std::string, Asset*> assets_;
	};

	class AssetIndexer
	{
	public:
		AssetIndexer(Asset* asset, time_t begin);
		AssetIndexer(Asset* asset);
		void MoveTo(time_t pos);
		time_t GetIndexTime();
		bool valid() { return index_ >= 0; }
		bool at_begin(){ return index_ == 0; }
		bool at_end() { return index_ == asset_->raw_data()->size() - 1; }
		Asset* asset() const { return asset_; }
		int index() const { return index_; }
	private:
		Asset* asset_;
		int index_;
	};

	typedef std::map<std::string, AssetIndexer> AssetIndexerMap;
	typedef std::map<std::string, AssetIndexer*> AssetIndexerPtrMap;
}

#endif