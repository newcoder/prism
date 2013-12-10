// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// Common data structure definitions.

#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <vector>

namespace prism {

	const double kEpsilon = 0.000001;
	const double kMacdThreshold = 0.000001;
	const int kMacdLookBack = 4;
	const std::string kBlockAllAShares = "ÆäËü°å¿é\\A¹É°å¿é";
	const std::string kStockPatterns = "patterns";
	const std::string kStockBlocks = "blocks";
	const double kCommissionRate = 0.001;
	const int kHand = 100;
	const double kDefaultAdjacentDelta = 0.02;
	const double kDefaultFixedDelta = 0.02;

	typedef enum
	{
		DATA_TYPE_DAILY = 0,
		DATA_TYPE_WEEKLY,
		DATA_TYPE_MONTHLY
	} DATA_TYPE;

	// High, Low, Open, Close prices and Volume 
	class HLOC {
	public:
		double high;
		double low;
		double open;
		double close;
		time_t time;
		double amount;
		int volume;
	};

	typedef enum {
		PRICE_TYPE_H = 1,
		PRICE_TYPE_L = 2,
		PRICE_TYPE_O = 3,
		PRICE_TYPE_C = 4,
	} PRICE_TYPE;

	// plain HLOC series
	typedef std::vector<HLOC> HLOCList;

	// plain double series
	typedef std::vector<double> DoubleList;

	template <typename T>
	class TimePoint 
	{
	public:
		TimePoint(){};
		TimePoint(T val, time_t pos) : value(val), position(pos) {}
		T value;
		time_t position;
	};

	typedef TimePoint<double> DoubleTimePoint;
	typedef std::vector<DoubleTimePoint> DoubleTimeList;

	// split options
	class SplitOptions
	{
	public:
		double dividend;
		double bonusShares;
		double rationedShares;
		double rationedPrice;
		time_t time;
	};

	typedef enum
	{
		TREND_UP = 1,
		TREND_DOWN,
		TREND_STABLE,
	} TREND_TYPE;

	class Trend
	{
	public:
		TREND_TYPE type;
		int period;
		int strength;
		// how to measure the strength?  to use the slope of the fitting line get from local min/max..
		int reliability;
	};

	// characteristics of a MACD/KDJ crossover
	class CrossOver
	{
	public:
		bool positive;
		double slope;
		double area;
		int length;
		double profit;  // in percentage
		time_t time;
	};

	typedef std::vector<CrossOver> CrossOverList;

}

#endif
