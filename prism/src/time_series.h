// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for time series processing.

#ifndef TIME_SERIES_H
#define TIME_SERIES_H

#include <vector>
#include <algorithm>
#include "time.h"
#include "common.h"

namespace prism {

	class IPattern;
	class ILocalIndicator;

	// linear regression coefficient. Y = A*X + B
	class LRCoef 
	{
	public:
		LRCoef(double a, double b): A(a), B(b) {}
		double A;
		double B;
	};

	// utility operations for DoubleTimeList
	class TLUtils
	{
	public:
		// unary operations		
		template <typename T>
		static std::shared_ptr<DoubleTimeList> UnaryOpHelper(std::shared_ptr<DoubleTimeList> tl, T functor)
		{
			std::for_each(tl->begin(), tl->end(), functor);
			return tl;
		}
		// binary operations, these functions are NOT responsible for memory allocation and release.
		template <typename T>
		static std::shared_ptr<DoubleTimeList> BinaryOpHelper(std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2, 
			std::shared_ptr<DoubleTimeList> result, T functor)
		{
			size_t count1 = tl1->size();
			size_t count2 = tl2->size();
			if (count1 != count2)
				return result;
			result->clear();
			DoubleTimeList::const_iterator cit1 = tl1->begin();
			DoubleTimeList::const_iterator cit2 = tl2->begin();
			while (cit1 < tl1->end())
			{
				result->push_back(DoubleTimePoint(functor((*cit1).value, (*cit2).value), (*cit1).position));
				cit1++;
				cit2++;
			}
			return result;
		}

	public:
		// populate from HLOC series
		static std::shared_ptr<DoubleTimeList> Populate(HLOCList::const_iterator cit_begin, HLOCList::const_iterator cit_end, 
			PRICE_TYPE type, std::shared_ptr<DoubleTimeList> result);
		static std::shared_ptr<DoubleTimeList> Populate(std::shared_ptr<HLOCList> hlocList, PRICE_TYPE type, std::shared_ptr<DoubleTimeList> result);
		// remove n elment from head or rear
		static void TLUtils::Remove(std::shared_ptr<DoubleTimeList> result, size_t num, bool head = true);
		// calculate the Euclidean distance between two time series
		static double Distance(std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2);
		// linear interpolate
		static double LinearInterpolate(const DoubleTimePoint& point1, const DoubleTimePoint& point2, time_t position);
	
		static std::shared_ptr<DoubleTimeList> Add(std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2, std::shared_ptr<DoubleTimeList> result);
		static std::shared_ptr<DoubleTimeList> Subtract(std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2, std::shared_ptr<DoubleTimeList> result);
		static std::shared_ptr<DoubleTimeList> Divide(std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2, std::shared_ptr<DoubleTimeList> result);
		static std::shared_ptr<DoubleTimeList> Multiply(std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2, std::shared_ptr<DoubleTimeList> result);
		
		// dump a single DoubleTimeList to csv file
		static void Dump(const std::string& file, std::shared_ptr<DoubleTimeList> tl);
		// dump two DoubleTimeLists to csv file, series2 is a subset of series1.
		static void Dump(const std::string& file, std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2);
		// dump three DoubleTimeLists to csv file, series2 is a subset of series1.
		static void Dump(const std::string& file, std::shared_ptr<DoubleTimeList> tl1, std::shared_ptr<DoubleTimeList> tl2, std::shared_ptr<DoubleTimeList> tl3);
	};

	// the time series class, encapsulate a set of operations for the time points in [begin, end)
	class TimeSeries
	{
	public:
		TimeSeries(DoubleTimeList::iterator begin, DoubleTimeList::iterator end);
		~TimeSeries();
	public:
		struct PointCompare {
			bool operator() (DoubleTimePoint p1, DoubleTimePoint p2) { return p1.value < p2.value; }
		} compareObj;
	public:
		// get the point with maximum value
		DoubleTimePoint GetMax() const;
		// get the point with minimum value
		DoubleTimePoint GetMin() const;
		// normalize values to 0.0-1.0
		void Normalize();
		// get the average 
		double Average() const;
		// get the deviation
		double Deviation() const;
		// find local maxs and mins
		int FindLocalExtremas(double threshold, std::shared_ptr<DoubleTimeList> result) const;
		// find turning points, to remove noise point in local extremas
		int FindTurningPoints(double thresholdP, double thresholdT, std::shared_ptr<DoubleTimeList> result) const;
		// match a given pattern
		int MatchPattern(std::shared_ptr<IPattern> pattern, std::shared_ptr<DoubleTimeList> result) const;
		// extract the pattern from range [begin, end], the function will return a new pattern, the caller need to release it
		std::shared_ptr<IPattern> ExtractPattern(int begin, int end, double fixed_delta, double adjacent_delta) const;
		// calculate indicators
		void CalculateIndicator(std::shared_ptr<ILocalIndicator> indicator) const;
		// do the linear regression for the time series
		LRCoef LinearRegression(std::shared_ptr<DoubleTimeList> result);
		// bottom-up piece wise, traditional method calcuate the cost to merge adjacent segment, we instead caculate the cost to remove individual point
		int PieceWise(double rate, std::shared_ptr<DoubleTimeList> result);
		// get count
		size_t Count() const { return end_ - begin_;  }
	private:
		bool CheckWindow(const DoubleTimeList::const_iterator& begin, const DoubleTimeList::const_iterator& end, double threshold, DoubleTimePoint& p) const;
	private:
		DoubleTimeList::iterator begin_;
		DoubleTimeList::iterator end_;
	};	
	
}

#endif