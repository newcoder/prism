// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for indicators, such as moving average,MACD, etc...

#ifndef INDICATORS_H
#define INDICATORS_H

#include "common.h"

namespace prism {

	class TimeSeries;

	// generic interface for all local indicator calculator
	class ILocalIndicator
	{
	public:
		virtual ~ILocalIndicator(){};
		// generate indicators for points in range [begin, end]
		virtual void Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end) {}
		virtual void Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end) {}
		// clear the results
		virtual void Clear() = 0;
	};

	// simple moving average
	class SMA : public ILocalIndicator
	{
	public:
		SMA(int period);
		virtual ~SMA();
		virtual void Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end);
		virtual void Clear();
		DoubleTimeList* result() const { return result_.get(); }
		int period() const { return period_; }
	protected:
		virtual bool Move(DoubleTimeList::const_iterator current, DoubleTimeList::const_iterator end);
	protected:
		int period_;
		std::unique_ptr<DoubleTimeList> result_;
	};

	// exponential moving average
	class EMA : public SMA
	{
	public:
		EMA(int period);
		EMA(int period, double weight);
		virtual ~EMA();
	private:
		virtual bool Move(DoubleTimeList::const_iterator current, DoubleTimeList::const_iterator end);
	private:
		double weight_;
	};

	// moving average convergence-divergence
	class MACD : public ILocalIndicator
	{
	public:
		MACD(int shortPeriod, int longPeriod, int signalPeriod);
		virtual ~MACD();
		virtual void Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end);
		virtual void Clear();
		void GenerateCrossOvers(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end);

		DoubleTimeList* macd();
		DoubleTimeList* signal();
		DoubleTimeList* histogram();
		CrossOverList* cross_overs();
	private:
		int long_period_;
		int short_period_;
		int signal_period_;
		std::unique_ptr<EMA> long_ema_;
		std::unique_ptr<EMA> short_ema_;
		std::unique_ptr<EMA> signal_ema_;
		std::unique_ptr<DoubleTimeList> macd_;
		std::unique_ptr<DoubleTimeList> histogram_;
		std::unique_ptr<CrossOverList> cross_overs_;
	};

	// relative strength index
	class RSI : public ILocalIndicator
	{
	public:
		RSI(int period);
		virtual ~RSI();
		virtual void Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end);
		virtual void Clear(); 
		DoubleTimeList* result() { return result_.get(); }
	private:
		int period_;
		std::unique_ptr<DoubleTimeList> result_;
	};

	// RSV, Raw Stochastic Value
	class RSV : public ILocalIndicator
	{
	public:
		RSV(int period);
		virtual ~RSV();
		virtual void Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end);
		virtual void Clear(); 
		DoubleTimeList* result() { return result_.get(); }
	private:
		int period_;
		std::unique_ptr<DoubleTimeList> result_;
	};

	// KDJ indicator
	class KDJ : public ILocalIndicator
	{
	public:
		KDJ(int period, int smooth1, int smooth2);
		virtual ~KDJ();
		virtual void Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end);
		virtual void Clear(); 
		DoubleTimeList* k();
		DoubleTimeList* d();
		DoubleTimeList* j();
	private:
		int period_;
		int smooth1_;
		int smooth2_;
		std::unique_ptr<RSV> rsv_;
		std::unique_ptr<EMA> k_ema_;
		std::unique_ptr<EMA> d_ema_;
		std::unique_ptr<DoubleTimeList> j_;
	};

	// Trend Line, by local min/max
	class TL : public ILocalIndicator
	{
	public:
		TL(double threshold);
		virtual ~TL();
		virtual void Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end);
		virtual void Clear();
		DoubleTimeList* short_trend() { return short_trend_.get(); }
		DoubleTimeList* medium_trend() { return medium_trend_.get(); }
	private:
		double threshold_;
		std::unique_ptr<DoubleTimeList> short_trend_;
		std::unique_ptr<DoubleTimeList> medium_trend_;
	};

	// Count Ratio
	// first, count how many points above / below current price in the given period, then calculate the ratio
	class CR : public ILocalIndicator
	{
	public:
		CR(int period);
		virtual ~CR();
		virtual void Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end);
		virtual void Clear();
		DoubleTimeList* result() { return result_.get(); }
	private:
		int period_;
		std::unique_ptr<DoubleTimeList> result_;
	};


}

#endif