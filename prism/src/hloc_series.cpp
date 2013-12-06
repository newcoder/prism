// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// routines and classes for hloc series processing.

#include "hloc_series.h"
#include "util.h"
#include "indicator.h"
#include "store.h"
#include <algorithm>

namespace prism 
{

	SplitRestorer::SplitRestorer(const SplitOptions& options): options_(options)
	{
	}
	
	void SplitRestorer::Restore(HLOC* point)
	{
		if (point->time < options_.time)
		{
			// forward
			point->high = RestoreForward(point->high);
			point->low = RestoreForward(point->low);
			point->open = RestoreForward(point->open);
			point->close = RestoreForward(point->close);
			point->volume = (int)RestoreBackward(point->volume);
		}
		else
		{
			// backward
			point->high = RestoreBackward(point->high);
			point->low = RestoreBackward(point->low);
			point->open = RestoreBackward(point->open);
			point->close = RestoreBackward(point->close);
			point->volume = (int)RestoreForward(point->volume);
		}
	}

	// 除权除息价＝（股权登记日的收盘价－每股所分红利现金额＋配股价×每股配股数）÷（1＋每股送红股数＋每股配股数）
	// formula for applying split:
	// after = (before - dividend + rationed price * rationed shares) / (1 + bonus shares + rationed shares)
	inline double SplitRestorer::RestoreForward(double price)
	{
		return (price - options_.dividend + options_.rationedPrice * options_.rationedShares) / (1 + options_.bonusShares + options_.rationedShares);
	}

	inline double SplitRestorer::RestoreBackward(double price)
	{
		return price * (1 + options_.bonusShares + options_.rationedShares) + options_.dividend - options_.rationedPrice * options_.rationedShares;
	}


	HLOCSeries::HLOCSeries(HLOCList::iterator begin, HLOCList::iterator end): begin_(begin), end_(end)
	{
	}

	HLOCSeries::~HLOCSeries()
	{
	}
	
	void HLOCSeries::RestoreSplit(const SplitOptions& options, bool forward)
	{
		SplitRestorer *restorer = new SplitRestorer(options);
		if (forward) {
			HLOCList::iterator it = begin_;
			while (it != end_) {
				HLOC point = *it;
				if (point.time >= options.time) break;
				restorer->Restore(&point);
				*it++ = point;
			}
		}
		else 
		{
			HLOCList::iterator it = begin_;
			while (it != end_) {
				HLOC point = *it;
				if (point.time < options.time) continue;
				restorer->Restore(&point);
				*it++ = point;
			}
		}
	}

	void HLOCSeries::Normalize()
	{
		struct PointCompare {
			bool operator() (HLOC p1,HLOC p2) { return p1.high < p2.high; }
		} compareObj;

		HLOC max = *std::max_element(begin_, end_, compareObj);

		struct PointNormalize {
			void operator() (HLOC& p) 
			{ 
				p.high = p.high / denominator;
				p.low = p.low / denominator;
				p.open = p.open / denominator;
				p.close = p.close / denominator;
			}
		   double denominator;
		} normalizeObj;

		normalizeObj.denominator = max.high;
		std::for_each(begin_, end_, normalizeObj);
	}

	HLOC HLOCSeries::ShrinkToOne(const HLOCList& list)
	{
		HLOC p;
		assert(list.size() > 0);
		p = list[0];
		size_t i = 1;
		while(i < list.size())
		{
			p.amount += list[i].amount;
			p.volume += list[i].volume;
			if (list[i].high > p.high)
				p.high = list[i].high;
			if (list[i].low < p.low)
				p.low = list[i].low;
			i++;
		}
		p.close = list[list.size() - 1].close;
		p.time = list[list.size() - 1].time;
		return p;
	}

	void HLOCSeries::ShrinkByNum(size_t num, HLOCList* result)
	{
		HLOCList group;
		HLOCList::iterator it = begin_;
		while (it != end_)
		{
			if (group.size() < num)
			{
				group.push_back(*it);
				it++;
			}
			else
			{
				result->push_back(ShrinkToOne(group));
				group.clear();
			}
		}
		if (!group.empty())
			result->push_back(ShrinkToOne(group));
	}

	template <typename T>
	void HLOCSeries::ShrinkByTime(HLOCList *result, T& functor)
	{
		HLOCList group;
		HLOCList::iterator it = begin_;
		time_t previous;
		while (it != end_)
		{
			if (group.empty())
			{
				previous = (*it).time;
				group.push_back(*it);
				it++;
			} 
			else if (functor(previous, (*it).time))
			{
				group.push_back(*it);
				it++;
			}
			else
			{
				result->push_back(ShrinkToOne(group));
				group.clear();
			}
		}
		if (!group.empty())
			result->push_back(ShrinkToOne(group));
	}

	void HLOCSeries::ShrinkByWeek(HLOCList* result)
	{
		struct InWeek {
			bool operator() (time_t d1, time_t d2) { return InSameWeek(d1, d2); }
		} sameWeek;

		ShrinkByTime(result, sameWeek);
	}

	void HLOCSeries::ShrinkByMonth(HLOCList* result)
	{
		struct InMonth {
			bool operator() (time_t d1, time_t d2) { return InSameMonth(d1, d2); }
		} sameMonth;
		ShrinkByTime(result, sameMonth);
	}

	void HLOCSeries::CalculateIndicator(ILocalIndicator* indicator) const
	{
		indicator->Generate(begin_, end_);
	}

}