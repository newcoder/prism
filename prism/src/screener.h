// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for screen assets based on rule

#ifndef SCREENER_H
#define SCREENER_H

#include "rule.h"
#include "asset.h"
#include <string>
#include <vector>

namespace prism {
	
	class Screener
	{
	public:
		Screener(IRule* rule) : rule_(rule) {}
		~Screener() {}
	public:
		void Screen(AssetIndexerList& asset_index_list, time_t pos, std::vector<int>* result);
	private:
		IRule *rule_;
	};


}

#endif