// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for hloc series processing.

#ifndef HLOC_SERIES_H
#define HLOC_SERIES_H

#include "time_series.h"

namespace prism {

	class ILocalIndicator;
	class IStore;

	class SplitRestorer 
	{
	public:
		SplitRestorer(const SplitOptions& options);
	public:
		void Restore(HLOC* point);
	private:
		inline double RestoreForward(double price);
		inline double RestoreBackward(double price);
	private:
		SplitOptions options_;
	};

	// the HLOC series
	class HLOCSeries
	{
	public:
		HLOCSeries(HLOCList::iterator begin, HLOCList::iterator end);
		~HLOCSeries();
	public:
		// load from iStore
		bool Load(const std::string& symbol, IStore* store);
		// restore the price from split, forward = true, means keep current price fixed, adjust the price before the split date
		void RestoreSplit(const SplitOptions& options, bool forward = true);
		// normalize all prices to 0.0-1.0
		void Normalize();
		// shrink
		void ShrinkByNum(size_t num, HLOCList *result);
		// shrink to weekly
		void ShrinkByWeek(HLOCList *result);
		// shrink to monthly
		void ShrinkByMonth(HLOCList *result);
		// calcuate local indicators
		void CalculateIndicator(ILocalIndicator* indicator) const;
	private:
		template <typename T>
			void ShrinkByTime(HLOCList *result, T& functor);
		HLOC ShrinkToOne(const HLOCList& list);
	private:
		HLOCList::iterator begin_;
		HLOCList::iterator end_;

	};

}

#endif