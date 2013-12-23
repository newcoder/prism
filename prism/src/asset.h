// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
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
	class Asset;

	class AssetScale
	{
	public:
		AssetScale(Asset* asset);
		~AssetScale();
		void Create(DATA_TYPE data_type, int data_num);
		void ClearIndicators();
		HLOCList* raw_data() const { return raw_data_; }
		std::string to_string() const {
			return std::to_string((int)data_type_) + '_' + std::to_string(data_num_);
		}
		DATA_TYPE data_type() const { return data_type_; }
		int data_num() const { return data_num_; }
		ILocalIndicator* indicators(const std::string& indicator_str);
	public:
		ILocalIndicator* GenerateIndicator(const std::string& indicator_str);
		static std::string ToString(DATA_TYPE data_type, int data_num);
	private:
		Asset* asset_;
		DATA_TYPE data_type_;
		int data_num_;
		HLOCList* raw_data_;
		std::map<std::string, ILocalIndicator*> indicators_;
	};

	class Asset
	{
	public:
		Asset(const std::string& symbol);
		~Asset();
		bool Load(IStore* store, size_t begin_year, size_t end_year);
		bool Update(IStore* store, size_t begin_year, size_t end_year);
		AssetScale* scales(DATA_TYPE data_type, int data_num);
		void ClearScales();
		HLOCList* raw_data() const { return raw_data_; }
		std::string symbol() const { return symbol_; }
		size_t begin_year() const { return begin_year_; }
		size_t end_year() const { return end_year_; }
	public:
		AssetScale* CreateScale(DATA_TYPE data_type, int data_num);
	private:
		std::string symbol_;
		size_t begin_year_;
		size_t end_year_;
		HLOCList* raw_data_;
		std::map<std::string, AssetScale*> scales_;
	};

	class AssetsProvider
	{
	public:
		AssetsProvider(IStore* store);
		~AssetsProvider();
		void Clear();
		int LoadAssets(const std::string& symbols_pattern, size_t begin_year, size_t end_year);
		int LoadAssets(const std::vector<std::string>& symbols, size_t begin_year, size_t end_year);
		bool LoadAll();
		Asset* Get(const std::string& symbol_string) const;
		std::map<std::string, Asset*>* assets() { return &assets_; }
	private:
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
		time_t GetIndexTime() const;
		bool GetIndexData(HLOC* data) const;
		bool valid() const { return index_ >= 0; }
		bool at_begin() const { return index_ == 0; }
		bool at_end() const { return index_ == asset_->raw_data()->size() - 1; }
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