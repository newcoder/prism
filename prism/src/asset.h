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
		bool Load(IStore* store, size_t begin, size_t end, DATA_TYPE data_type = DATA_TYPE_DAILY, int data_num = 1);
		HLOCList* raw_data() { return raw_data_; }
		std::string symbol() { return symbol_; }
		ILocalIndicator* indicators(const std::string& indicator_str);
	private:
		ILocalIndicator* GenerateIndicator(const std::string& indicator_str);
	private:
		std::string symbol_;
		HLOCList* raw_data_;
		std::map<std::string, ILocalIndicator*> indicators_;
	};

	class AssetsLoader
	{
	public:
		AssetsLoader(IStore* store);
		~AssetsLoader();
		void Clear();
		int LoadAssets(const std::vector<std::string>& symbols, size_t begin, size_t end, DATA_TYPE data_type = DATA_TYPE_DAILY, int data_num = 1);
		Asset* Get(const std::string& symbol);
		std::map<std::string, Asset*>* assets() { return &assets_; }
	private:
		IStore* store_;
		std::map<std::string, Asset*> assets_;
	};
}

#endif