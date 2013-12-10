// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for indicators, such as moving average,MACD, etc...

#include "indicator.h"
#include "time_series.h"
#include "hloc_series.h"
#include <iostream>
#include <cassert>

namespace prism 
{
	SMA::SMA(int period): period_(period)
	{
		result_ = new DoubleTimeList();
		//std::cout << "SMA::SMA()" << std::endl;

	}

	SMA::~SMA()
	{
		delete result_;
		//std::cout << "SMA::~SMA()" << std::endl;

	}

	void SMA::Clear()
	{
		result_->clear();
	}

	void SMA::Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end)
	{
		auto cit = begin;
		Clear();
		while(cit != end)
		{
			if (!Move(cit, end))
				break;
			cit++;
		}
	}

	bool SMA::Move(DoubleTimeList::const_iterator current, DoubleTimeList::const_iterator end)
	{
		int i = 0;
		double sum = 0.0f;
		while(current != end && i < period_)
		{
			sum += (*current).value;
			current++;
			i++;
		}
		current--;		
		if (i < period_ || (current == end))
		{
			// points not enough
			return false;
		}		

		result_->push_back(DoubleTimePoint(sum / period_, (*current).position));
		return true;
	}

	EMA::EMA(int period): SMA(period)
	{
		weight_ = 2 / ((double)(period_) + 1);
	}

	EMA::EMA(int period, double weight): SMA(period), weight_(weight)
	{
		//std::cout << "EMA::EMA()" << std::endl;

	}

	EMA::~EMA()
	{
		//std::cout << "EMA::~EMA()" << std::endl;

	}

	bool EMA::Move(DoubleTimeList::const_iterator current, DoubleTimeList::const_iterator end)
	{
		size_t count = result_->size();
		if (count == 0)
		{
			// use SMA to calculate the first point
			return SMA::Move(current + 1, end);
		}
		else
		{
			// EMA: {Close - EMA(previous day)} x multiplier + EMA(previous day)
			int i = 0;
			while(current != end && i < period_)
			{
				current++;
				i++;
			}
			if (i < period_ || (current == end))
			{
				// points not enough
				return false;
			}
			double previous = result_->at(count - 1).value;
			result_->push_back(DoubleTimePoint(((*current).value - previous)*weight_ + previous, (*current).position));
			return true;
		}
	}

	MACD::MACD(int shortPeriod, int longPeriod, int signalPeriod): short_period_(shortPeriod), long_period_(longPeriod), signal_period_(signalPeriod)
	{
		long_ema_ = new EMA(long_period_);
		short_ema_ = new EMA(short_period_);
		signal_ema_ = new EMA(signal_period_);
		macd_ = new DoubleTimeList();
		histogram_ = new DoubleTimeList();
		cross_overs_ = new CrossOverList();

		//std::cout << "MACD::MACD()" << std::endl;
	}

	MACD::~MACD()
	{
		delete long_ema_;
		delete short_ema_;
		delete signal_ema_;
		delete macd_;
		delete histogram_;
		delete cross_overs_;
		//std::cout << "MACD::~MACD()" << std::endl;

	}
	
	void MACD::Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end)
	{
		// generate long EMA
		long_ema_->Generate(begin, end);
		DoubleTimeList* long_result = long_ema_->result();
		// generate short EMA
		short_ema_->Generate(begin, end);
		DoubleTimeList* short_result = short_ema_->result();
		// get the MACD by differing the long and short EMA
		TLUtils::Remove(short_result, short_result->size() - long_result->size());
		TLUtils::Subtract(short_result, long_result, macd_);
		// smooth the MACD
		TimeSeries ts(macd_->begin(), macd_->end());
		ts.CalculateIndicator(signal_ema_);
		// differ the MACD and the smoothed MACD
		TLUtils::Remove(macd_, macd_->size() - signal_ema_->result()->size());
		TLUtils::Subtract(macd_, signal_ema_->result(), histogram_);
		// calculate crossovers
		GenerateCrossOvers(begin, end);
	}

	void MACD::GenerateCrossOvers(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end)
	{
		cross_overs_->clear();
		auto cit = histogram_->begin();
		auto cit_price = begin;

		if (cit == histogram_->end())
			return;
		CrossOver co;
		DoubleTimePoint previous = *cit;
		DoubleTimePoint previous_price = *cit_price;
		co.area = 0.0F;
		co.positive = (*cit).value < 0;
		co.length = 0;
		co.slope = 0.0F;
		co.profit = 0.0F;
		co.time = (*cit).position;
		// push the first point 
		cross_overs_->push_back(co);
		// move the price iterator to crossover position
		while (cit_price != end)
		{
			if (co.time == (*cit_price).position)
				break;
			cit_price++;
		}
		previous_price = *cit_price;

		co.positive = !co.positive;
		while (cit != histogram_->end())
		{
			double value = (*cit).value;
			bool sameSign = co.positive ? value > 0: value < 0;
			if (sameSign)
			{
				co.length++;
				co.area = co.area + value;
				co.time = (*cit).position;
				co.slope = std::fabs((previous.value - value) / (previous.value + value));
				previous = *cit;
				cit++;
			}
			else
			{
				while (cit_price != end)
				{
					if (co.time == (*cit_price).position)
						break;
					cit_price++;
				}
				co.profit = ((*cit_price).value - previous_price.value) / previous_price.value;
				previous_price = *cit_price;
				cross_overs_->push_back(co);
				// new crossover
				co.area = 0.0F;
				co.positive = (*cit).value > 0;
				co.length = 0;
				co.slope = 0.0F;
				co.time = (*cit).position;
			}
		}
		// push the last point, the first & last point is not real crossover.
		while (cit_price != end)
		{
			if (co.time == (*cit_price).position)
				break;
			cit_price++;
		}
		co.profit = ((*cit_price).value - previous_price.value) / previous_price.value;
		previous_price = *cit_price;
		cross_overs_->push_back(co);
	}

	DoubleTimeList* MACD::macd()
	{
		return macd_;
	}
	
	DoubleTimeList* MACD::signal()
	{
		return signal_ema_->result();
	}
	
	DoubleTimeList* MACD::histogram()
	{
		return histogram_;
	}

	CrossOverList* MACD::cross_overs()
	{
		return cross_overs_;
	}

	void MACD::Clear()
	{
		long_ema_->Clear();
		short_ema_->Clear();
		signal_ema_->Clear();
		macd_->clear();
		histogram_->clear();
		cross_overs_->clear();
	}

	RSI::RSI(int period) : period_(period)
	{
		result_ = new DoubleTimeList();
	}

	RSI::~RSI()
	{
		delete result_;
	}

	void RSI::Generate(DoubleTimeList::const_iterator begin, DoubleTimeList::const_iterator end)
	{
		auto cit = begin;
		DoubleTimeList gain, loss;
		SMA gain_ma(period_), loss_ma(period_); // google finance use SMA to smooth, may use EMA...
		double previous = (*cit).value;
		cit++;
		while (cit != end)
		{
			if ((*cit).value > previous)
			{
				gain.push_back(DoubleTimePoint((*cit).value - previous, (*cit).position));
				loss.push_back(DoubleTimePoint(0.0F, (*cit).position));
			}
			else
			{
				loss.push_back(DoubleTimePoint(previous - (*cit).value,(*cit).position));
				gain.push_back(DoubleTimePoint(0.0F, (*cit).position));
			}
			previous = (*cit).value;
			cit++;
		}

		TimeSeries gain_ts(gain.begin(), gain.end());
		gain_ts.CalculateIndicator(&gain_ma);
		TimeSeries loss_ts(loss.begin(), loss.end());
		loss_ts.CalculateIndicator(&loss_ma);
		TLUtils::Divide(gain_ma.result(), loss_ma.result(), result_);

		struct RSIndex {
			void operator() (DoubleTimePoint& p) { p.value = 100 - 100 / (1 + p.value); }
		} obj;

		TLUtils::UnaryOpHelper(result_, obj);
		
	}

	void RSI::Clear()
	{
		result_->clear();
	}

	RSV::RSV(int period): period_(period)
	{
		result_ = new DoubleTimeList();
	}

	RSV::~RSV()
	{
		delete result_;
	}

	void RSV::Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end)
	{
		auto it = begin;
		if (end - begin < period_)
			return;
		
		size_t n = period_ - 1;

		while (it + n != end)
		{
			double Hn = (*it).high;
			double Ln = (*it).low;
			double Cn = (*(it + n)).close;
			for (int i = 0; i < period_; i++)
			{
				if ((*(it + i)).high > Hn)
					Hn = (*(it + i)).high;
				if ((*(it + i)).low < Ln)
					Ln = (*(it + i)).low;
			}
			double rsv = (Cn - Ln)*100 / (Hn - Ln);
			result_->push_back(DoubleTimePoint(rsv, (*(it + n)).time));
			it++;
		}
	}

	void RSV::Clear()
	{
		result_->clear();
	}


	KDJ::KDJ(int period, int smooth1, int smooth2): period_(period), smooth1_(smooth1), smooth2_(smooth2)
	{
		rsv_ = new RSV(period_);
		k_ema_ = new EMA(smooth1, ((double)smooth1 - 1) /(smooth1 + 1));
		d_ema_ = new EMA(smooth2, ((double)smooth2 - 1) /(smooth2 + 1));
		j_ = new DoubleTimeList();
	}

	KDJ::~KDJ()
	{
		delete rsv_;
		delete k_ema_;
		delete d_ema_;
		delete j_;
	}

	void KDJ::Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end)
	{
		// generate RSV from HLOC series
		rsv_->Generate(begin, end);
		DoubleTimeList *rsv = rsv_->result();
		// generate K and D from RSV
		k_ema_->Generate(rsv->begin(), rsv->end());
		d_ema_->Generate(k()->begin(), k()->end());
		// generate J = 3D - 2K // or J = 3K - 2D ???
		TLUtils::Remove(k(), k()->size() - d()->size());
		struct Jcalculator {
			double operator() (double p1, double p2) { return 3*p2 -2*p1; }
		} Jcalc;

		TLUtils::BinaryOpHelper(k(), d(), j_, Jcalc);
	}

	DoubleTimeList* KDJ::k() 
	{
		return k_ema_->result(); 
	}
	
	DoubleTimeList* KDJ::d() 
	{ 
		return d_ema_->result(); 
	}

	DoubleTimeList* KDJ::j() 
	{ 
		return j_; 
	}

	void KDJ::Clear()
	{
		rsv_->Clear();
		k_ema_->Clear();
		d_ema_->Clear();
		j_->clear();
	}

	TL::TL(double threshold) : threshold_(threshold)
	{
		short_trend_ = new DoubleTimeList();
		medium_trend_ = new DoubleTimeList();
	}

	TL::~TL()
	{
		delete short_trend_;
		delete medium_trend_;
	}

	void TL::Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end)
	{
		DoubleTimeList tl;
		TLUtils::Populate(begin, end, PRICE_TYPE_H, &tl);
		TimeSeries ts(tl.begin(), tl.end());

		// find local min/max
		DoubleTimeList local_maxs;
		ts.FindLocalExtremas(threshold_, &local_maxs);

		// TODO....
	}
	
	void TL::Clear()
	{
		short_trend_->clear();
		medium_trend_->clear();
	}

	CR::CR(int period) : period_(period)
	{
		result_ = new DoubleTimeList();
	}

	CR::~CR()
	{
		delete result_;
	}

	void CR::Generate(HLOCList::const_iterator begin, HLOCList::const_iterator end)
	{
		auto it = begin;

		while (it != end)
		{	
			HLOC hloc_current = *it;	
			size_t num_above = 1; // initialize to 1, include itself.
			size_t num_below = 1;
			for (int i = 1; i <= period_; i++)
			{
				if (it >= begin + i)
				{
					HLOC hloc = *(it - i);
					if (hloc.low >= hloc_current.high)
						num_above++;
					else if (hloc.high <= hloc_current.low)
						num_below++;
					else
					{
						if (hloc.high > hloc_current.high)
							num_above++;
						if (hloc.low < hloc_current.low)
							num_below++;
					}
				}
			}
			double ratio = (double)num_above / (num_above + num_below);
			result_->push_back(DoubleTimePoint(ratio, (*(it)).time));
			it++;
		}
	}

	void CR::Clear()
	{
		result_->clear();
	}

}