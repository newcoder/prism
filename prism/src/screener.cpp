// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// for screener implementation.

#include "screener.h"

namespace prism {
	
	void Screener::Screen(AssetIndexerList& asset_index_list, std::vector<int>* result, time_t pos)
	{
		result->clear();
		for (unsigned i = 0; i < asset_index_list.size(); i++)
		{
			if (pos < 0 || asset_index_list[i].GetIndexTime() == pos)
			{
				if (rule_->Verify(asset_index_list[i]))
					result->push_back(i);
			}
		}
	}

}