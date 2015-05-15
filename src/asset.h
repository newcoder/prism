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
		AssetScale(const std::shared_ptr<Asset>& asset);
		~AssetScale();
		void Create(DATA_TYPE data_type, int data_num);
		void ClearIndicators();
		HLOCList* raw_data() const { return raw_data_.get(); }
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
		std::shared_ptr<Asset> asset_;
		DATA_TYPE data_type_;
		int data_num_;
		std::shared_ptr<HLOCList> raw_data_;
		std::map<std::string, std::unique_ptr<ILocalIndicator>> indicators_;
	};

	class Asset : public std::enable_shared_from_this<Asset>
	{
	public:
		Asset(const std::string& symbol);
		~Asset();
		bool Load(IStore* store, size_t begin_year, size_t end_year);
		bool Update(IStore* store, size_t begin_year, size_t end_year);
		std::shared_ptr<AssetScale> scales(DATA_TYPE data_type, int data_num);
		std::shared_ptr<AssetScale> base_scale() { return scales(DATA_TYPE_DAILY, 1); }
		void ClearScales();
		std::shared_ptr<HLOCList> raw_data() { return raw_data_; }
		std::string symbol() const { return symbol_; }
		size_t begin_year() const { return begin_year_; }
		size_t end_year() const { return end_year_; }
	public:
		std::shared_ptr<AssetScale> CreateScale(DATA_TYPE data_type, int data_num);
	private:
		std::string symbol_;
		size_t begin_year_;
		size_t end_year_;
		std::shared_ptr<HLOCList> raw_data_;
		std::map<std::string, std::shared_ptr<AssetScale>> scales_;
	};

	class AssetsProvider
	{
	public:
		AssetsProvider(const std::shared_ptr<IStore>& store);
		~AssetsProvider();
		void Clear();
		int LoadAssets(const std::string& symbols_pattern, size_t begin_year, size_t end_year);
		int LoadAssets(const std::vector<std::string>& symbols, size_t begin_year, size_t end_year);
		int LoadAll();
		std::shared_ptr<Asset> Get(const std::string& symbol_string) const;
		std::map<std::string, std::shared_ptr<Asset>>& assets() { return assets_; }
	private:
		bool ParseSymbolsPattern(const std::string& symbols_pattern, std::vector<std::string>& symbols);
	private:
		std::shared_ptr<IStore> store_;
		std::map<std::string, std::shared_ptr<Asset>> assets_;
	};

	class AssetScaleIndexer
	{
	public:
		AssetScaleIndexer(const std::shared_ptr<AssetScale>& scale, time_t begin);
		AssetScaleIndexer(const std::shared_ptr<AssetScale>& scale);
		void MoveTo(time_t pos) { ToBegin(); ForwardTo(pos); }
		void ForwardTo(time_t pos);
		void Forward() { if (!at_end()) index_++; }
		void Backward() { if (!at_begin()) index_--; }
		void ToBegin() { index_ = 0; }
		void ToEnd() { index_ = scale_->raw_data()->size() - 1; }
		time_t GetIndexTime() const;
		bool GetIndexData(HLOC& data) const;
		bool below_upper_limit() const;
		bool above_lower_limit() const;
		bool valid() const { return index_ >= 0; }
		bool at_begin() const { return index_ == 0; }
		bool at_end() const { return index_ == scale_->raw_data()->size() - 1; }
		std::shared_ptr<AssetScale> scale() const { return scale_; }
		int index() const { return index_; }
	private:
		std::shared_ptr<AssetScale> scale_;
		int index_;
	};

	class AssetIndexer
	{
	public:
		AssetIndexer(const std::shared_ptr<Asset>& asset, time_t begin);
		AssetIndexer(const std::shared_ptr<Asset>& asset);
	public:
		void MoveTo(time_t pos) { ToBegin(); ForwardTo(pos); }
		void ForwardTo(time_t pos);
		void Forward();
		void Backward();
		void ToBegin();
		void ToEnd();
		std::shared_ptr<Asset> asset() const { return asset_; }
		time_t GetIndexTime() const { return base_scale_indexer_->GetIndexTime(); }
		bool GetIndexData(HLOC& data) const { return base_scale_indexer_->GetIndexData(data); }
		bool below_upper_limit() const { return base_scale_indexer_->below_upper_limit(); }
		bool above_lower_limit() const { return base_scale_indexer_->above_lower_limit(); }
		bool valid() const { return base_scale_indexer_->valid(); }
		bool at_begin() const { return base_scale_indexer_->at_begin(); }
		bool at_end() const { return base_scale_indexer_->at_end(); }
		int index() const { return base_scale_indexer_->index(); }
		std::shared_ptr<AssetScaleIndexer> scale_indexers(DATA_TYPE data_type, int data_num);
		std::shared_ptr<AssetScaleIndexer> scale_indexers(const std::shared_ptr<AssetScale>& scale);
	private:
		std::shared_ptr<Asset> asset_;
		std::shared_ptr<AssetScaleIndexer> base_scale_indexer_;
		std::map<AssetScale*, std::shared_ptr<AssetScaleIndexer>> scale_indexers_;
	};

	typedef std::vector<AssetIndexer> AssetIndexerList;

	typedef std::map<std::string, AssetIndexer> AssetIndexerMap;
	typedef std::map<std::string, AssetIndexer*> AssetIndexerPtrMap;
}

#endif