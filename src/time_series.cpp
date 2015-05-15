// Copyright 2013, QT Inc.
// All righTimeSeries reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for time series processing.

#include "time_series.h"
#include "pattern.h"
#include "indicator.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>

#define EIGEN2_SUPPORT
#include <Eigen/Core>
#include <Eigen/LeastSquares>


namespace prism
{
	////////////////////////////////////////////////////////////////////////
	void TLUtils::Dump(const std::string& file, DoubleTimeList* tl)
	{
		std::ofstream of;
		of.open(file, std::ios::out);
		size_t count = tl->size();
		for (size_t i = 0; i < count; ++i)
		{
			DoubleTimePoint point = tl->at(i);
			of << TimeToString(point.position, "%Y-%m-%d %H:%M:%S") << "," << point.value << std::endl;
		}
		of.close();
	}

	void TLUtils::Dump(const std::string& file, DoubleTimeList* tl1, DoubleTimeList* tl2)
	{
		std::ofstream of;
		of.open(file, std::ios::out);
		size_t j = 0;
		size_t count1 = tl1->size();
		size_t count2 = tl2->size();
		for (size_t i = 0; i < count1; ++i)
		{
			DoubleTimePoint point1 = tl1->at(i);
			
			// get the value from series2 by linear interpolation
			DoubleTimePoint p = tl2->at(j);
			while (j < count2 && p.position < point1.position)
			{
				p = tl2->at(++j);
			}

			double point2value;
			if (p.position == point1.position) 
			{
				point2value = p.value;
			}
			else
			{
				DoubleTimePoint p1;
				p1 = tl2->at(j-1);
				point2value = p1.value + (p.value - p1.value)*(point1.position - p1.position)/(p.position - p1.position);
			}

			of << TimeToString(point1.position, "%Y-%m-%d %H:%M:%S") << "," << point1.value << "," << point2value << std::endl;
		}
		of.close();
	}

	void TLUtils::Dump(const std::string& file, DoubleTimeList* tl1, 
		DoubleTimeList* tl2, DoubleTimeList* tl3)
	{
		std::ofstream of;
		of.open(file, std::ios::out);
		size_t j = 0, k = 0;
		size_t count1 = tl1->size();
		size_t count2 = tl2->size();
		size_t count3 = tl3->size();
		for (size_t i = 0; i < count1; ++i)
		{
			DoubleTimePoint point1 = tl1->at(i);
			
			// get the value from series2 by linear interpolation
			DoubleTimePoint p = tl2->at(j);
			while (j < count2 && p.position < point1.position)
			{
				p = tl2->at(++j);
			}

			double point2value;
			if (p.position == point1.position) 
			{
				point2value = p.value;
			}
			else
			{
				DoubleTimePoint p1;
				p1 = tl2->at(j-1);
				point2value = p1.value + (p.value - p1.value)*(point1.position - p1.position)/(p.position - p1.position);
			}

			// get the value from series3 by linear interpolation
			p = tl3->at(k);
			while (k < count3 && p.position < point1.position)
			{
				p = tl3->at(++k);
			}

			double point3value;
			if (p.position == point1.position) 
			{
				point3value = p.value;
			}
			else
			{
				DoubleTimePoint p1;
				p1 = tl3->at(k-1);
				point3value = p1.value + (p.value - p1.value)*(point1.position - p1.position)/(p.position - p1.position);
			}

			of << TimeToString(point1.position, "%Y-%m-%d %H:%M:%S") << "," << point1.value << "," << point2value << "," << point3value << std::endl;
		}
		of.close();
	}

	double TLUtils::LinearInterpolate(const DoubleTimePoint& point1, const DoubleTimePoint& point2, time_t position)
	{
		return point1.value + (point2.value - point1.value)*(position - point1.position) / (point2.position - point1.position);
	}

	double TLUtils::Distance(DoubleTimeList* tl1, DoubleTimeList* tl2)
	{
		size_t count1, count2;
		count1 = tl1->size();
		count2 = tl2->size();

		if (count1 < 1 || count2 < 1)
			return -1.0F;

		// make sure both series have same begin position and end position
		DoubleTimePoint p1 = tl1->at(0);
		DoubleTimePoint p2 = tl2->at(0);
		assert(p1.position == p2.position);

		p1 = tl1->at(count1 - 1);
		p2 = tl2->at(count2 - 1);
		assert(p1.position == p2.position);

		size_t i1 = 0, i2 = 0;
		double sum = 0.0F;
		while (i1 < count1 && i2 < count2)
		{
			p1 = tl1->at(i1);
			p2 = tl2->at(i2);
			if (p1.position == p2.position)
			{
				sum += std::pow(p1.value - p2.value, 2);
				i1++;
				i2++;
			}
			if (p1.position < p2.position)
			{
				double inter_value = LinearInterpolate(tl2->at(i2 - 1), tl2->at(i2), p1.position);
				sum += std::pow(p1.value - inter_value, 2);
				i1++;
			}
			if (p1.position > p2.position)
			{
				double inter_value = LinearInterpolate(tl1->at(i1 - 1), tl1->at(i1), p2.position);
				sum += std::pow(inter_value - p2.value, 2);
				i2++;
			}
		}

		return std::sqrt(sum);
	}

	DoubleTimeList* TLUtils::Add(DoubleTimeList* tl1, DoubleTimeList* tl2, DoubleTimeList* result)
	{
		struct Adder {
			double operator() (double p1, double p2) { return p1 + p2; }
		} adder;

		return BinaryOpHelper(tl1, tl2, result, adder);
	}

	DoubleTimeList* TLUtils::Subtract(DoubleTimeList* tl1, DoubleTimeList* tl2, DoubleTimeList* result)
	{
		struct Subtracter {
			double operator() (double p1, double p2) { return p1 - p2; }
		} subtracter;

		return BinaryOpHelper(tl1, tl2, result, subtracter);
	}

	DoubleTimeList* TLUtils::Multiply(DoubleTimeList* tl1, DoubleTimeList* tl2, DoubleTimeList* result)
	{
		struct Multiplyer {
			double operator() (double p1, double p2) { return p1 * p2; }
		} multiplyer;

		return BinaryOpHelper(tl1, tl2, result, multiplyer);
	}

	DoubleTimeList* TLUtils::Divide(DoubleTimeList* tl1, DoubleTimeList* tl2, DoubleTimeList* result)
	{
		struct Divider {
			double operator() (double p1, double p2) { return p1 / p2; }
		} divider;

		return BinaryOpHelper(tl1, tl2, result, divider);
	}

	DoubleTimeList* TLUtils::Populate(HLOCList::const_iterator cit_begin, HLOCList::const_iterator cit_end, PRICE_TYPE type, DoubleTimeList* result)
	{
		auto citBegin = cit_begin;
		result->clear();
		while (citBegin != cit_end)
		{
			DoubleTimePoint point;
			point.position = (*citBegin).time;
			switch (type)
			{
			case PRICE_TYPE_H:
				point.value = (*citBegin).high;
				break;
			case PRICE_TYPE_L:
				point.value = (*citBegin).low;
				break;
			case PRICE_TYPE_O:
				point.value = (*citBegin).open;
				break;
			case PRICE_TYPE_C:
				point.value = (*citBegin).close;
				break;
			default:
				break;
			}
			result->push_back(point);
			citBegin++;
		}
		return result;
	}

	DoubleTimeList* TLUtils::Populate(HLOCList* hlocList, PRICE_TYPE type, DoubleTimeList* result)
	{
		return TLUtils::Populate(hlocList->begin(), hlocList->end(), type, result);
	}

	void TLUtils::Remove(DoubleTimeList* result, size_t num, bool head)
	{
		if (num > result->size())
			return;
		if (head)
		{
			result->erase(result->begin(), result->begin() + num);
		}
		else
		{
			result->erase(result->end() - num, result->end());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	TimeSeries::TimeSeries(DoubleTimeList::iterator begin, DoubleTimeList::iterator end) : begin_(begin), end_(end)
	{

	}

	TimeSeries::~TimeSeries()
	{

	}

	DoubleTimePoint TimeSeries::GetMax() const
	{
		return *std::max_element(begin_, end_, compareObj);
	}

	DoubleTimePoint TimeSeries::GetMin() const
	{
		return *std::min_element(begin_, end_, compareObj);
	}

	void TimeSeries::Normalize()
	{
		struct PointNormalize {
			void operator() (DoubleTimePoint& p) { p.value = p.value / denominator; }
			double denominator;
		} obj;

		obj.denominator = GetMax().value;
		std::for_each(begin_, end_, obj);
	}
		
	double TimeSeries::Average() const
	{
		double sum = 0.0F;
		auto cit = begin_;
		while (cit != end_)
		{
			sum += (*cit).value;
			cit++;
		}
		return sum / (double)(Count());
	}
	
	double TimeSeries::Deviation() const
	{
		double miu = Average();
		double sum = 0.0F;
		auto cit = begin_;
		while (cit != end_)
		{
			sum += std::pow(((*cit).value - miu), 2);
			cit++;
		}
		return std::sqrt(sum / (double)(Count()));
	}

	int TimeSeries::FindLocalExtremas(double threshold, DoubleTimeList* result) const
	{
		DoubleTimeList::const_iterator window_begin, window_end;
		size_t count = Count();
		if (count < 3) return 0;

		result->clear();
		// add the first element
		result->push_back(*begin_);
		window_begin = begin_;
		window_end = window_begin + 2;
		while (window_end != end_)
		{
			DoubleTimePoint p;
			bool hasExtrema = CheckWindow(window_begin, window_end, threshold, p);
			if (hasExtrema)
			{
				result->push_back(p);
				while ((*window_begin++).position != p.position);
				--window_begin;
			}
			window_end++;
		}
		result->push_back(*(end_ - 1));
		return result->size();
	}

	bool TimeSeries::CheckWindow(const DoubleTimeList::const_iterator& begin, const DoubleTimeList::const_iterator& end, double threshold, DoubleTimePoint& p) const
	{
		p = *std::max_element(begin, end + 1, compareObj);
		if (std::fabs(p.value - (*begin).value) / (p.value + (*begin).value) > threshold
			&& std::fabs(p.value - (*end).value) / (p.value + (*end).value) > threshold)
		{
			// find a maxima
			return true;
		}
		else
		{
			// try to find a minima
			p = *std::min_element(begin, end + 1, compareObj);
			return std::fabs(p.value - (*begin).value) / (p.value + (*begin).value) > threshold
				&& std::fabs(p.value - (*end).value) / (p.value + (*end).value) > threshold;
		}
	}

#define FILL_UNIT \
	{ \
	while (cit != end_ && unit.size() < 3) \
	unit.push_back(*cit++); \
	cit--; \
	} while (0); \

	int TimeSeries::FindTurningPoints(double thresholdP, double thresholdT, DoubleTimeList* result) const
	{
		size_t count = Count();
		if (count < 4)
			return 0;

		result->clear();

		auto cit = begin_;
		DoubleTimeList unit;

		// keep the first point and last point in
		result->push_back(*cit++);
		// initialize the first unit
		FILL_UNIT

		while (unit.size() == 3)
		{
			double p1 = unit[0].value;
			time_t t1 = unit[0].position;
			double p2 = unit[1].value;
			time_t t2 = unit[1].position;
			double p3 = unit[2].value;
			time_t t3 = unit[2].position;

			double dp1 = std::fabs((p1 - p2) / (p1 + p2));
			double dp2 = std::fabs((p2 - p3) / (p2 + p3));
			time_t dt1 = std::abs(t1 - t2);
			time_t dt2 = std::abs(t2 - t3);

			if ((dp1 > thresholdP && dp2 > thresholdP)
				|| (dp1 > thresholdP && dp2 < thresholdP && dt2 > thresholdT)
				|| (dp1 < thresholdP && dp2 > thresholdP && dt1 > thresholdT))
			{
				// case 1
				result->push_back(unit[0]);
				result->push_back(unit[1]);
				unit.clear();
				FILL_UNIT
			}
			else if (dp1 > thresholdP && dp2 < thresholdP)
			{
				// case 2
				cit++;
				if (cit == end_)
				{
					result->push_back(unit[0]);
					result->push_back(unit[1]);
					result->push_back(unit[2]);
					unit.clear();
				}
				else if (cit + 1 == end_) //i+3 is the last point
				{
					// skip i+2, add last point i+3
					result->push_back(unit[0]);
					result->push_back(unit[1]);
					result->push_back(*cit);
					unit.clear();
				}
				else
				{
					if (((*cit).value - unit[1].value)*(unit[1].value - unit[0].value) > 0)
					{
						unit.pop_back();
						unit.pop_back();
						FILL_UNIT
					}
					else
					{
						unit.pop_back();
						cit++;
						FILL_UNIT
					}
				}
			}
			else if (dp1 < thresholdP && dp2 > thresholdP)
			{
				// case 3
				unit.clear();
				FILL_UNIT
			}
			else if (dp1 < thresholdP && dp2 < thresholdP)
			{
				// case 4
				if (cit + 1 == end_)
				{
					// i+2 is the last point
					result->push_back(unit[0]);
					result->push_back(unit[1]);
					result->push_back(unit[2]);
					unit.clear();
				}
				else if (dt1 > thresholdT || dt2 > thresholdT)
				{
					unit.pop_back();
					unit.pop_back();
					FILL_UNIT
				}
				else
				{
					if (((*cit).value - unit[0].value)*(unit[1].value - unit[0].value) > 0)
					{
						unit.pop_back();
						unit.pop_back();
						cit++;
						FILL_UNIT
					}
					else
					{
						unit.clear();
						FILL_UNIT
					}
				}
			}
		}
		size_t i = 0;
		while (i < unit.size())
		{
			result->push_back(unit[i++]);
		}

		return result->size();
	}

	int TimeSeries::MatchPattern(IPattern* pattern, DoubleTimeList* result) const
	{
		result->clear();
		auto cit = begin_;
		while (cit != end_)
		{
			if (pattern->Match(cit, end_))
			{
				result->push_back(*cit);
			}
			cit++;
		}
		return result->size();
	}

	std::unique_ptr<Pattern> TimeSeries::ExtractPattern(int begin, int end, double fixed_delta, double adjacent_delta) const
	{
		auto pattern = std::make_unique<Pattern>(fixed_delta, adjacent_delta);
		// generate the pattern from the given segment
		pattern->Generate(begin_ + begin, begin_ + end);
		return pattern;
	}

		
	void TimeSeries::CalculateIndicator(ILocalIndicator* indicator) const
	{
		indicator->Generate(begin_, end_);
	}

	LRCoef TimeSeries::LinearRegression(DoubleTimeList* result)
	{
		int count = Count();

		Eigen::Vector2d *points = new Eigen::Vector2d[count];

		for (int i = 0; i < count; i++)
		{
			points[i] = Eigen::Vector2d((*(begin_ + i)).position, (*(begin_ + i)).value);
		}
		std::vector<Eigen::Vector2d*> points_ptrs(count);
		for (int k = 0; k < count; ++k) points_ptrs[k] = &points[k];
		Eigen::Vector2d coeffs; // will store the coefficienTimeSeries a, b, c
		Eigen::linearRegression(
			count,
			&(points_ptrs[0]),
			&coeffs,
			1 // the coord to express as a function of
			// the other ones. 0 means x, 1 means y, 2 means z.
			);
		delete[]points;

		result->clear();
		result->push_back(DoubleTimePoint((*begin_).position * coeffs.x() + coeffs.y(), (*begin_).position));
		result->push_back(DoubleTimePoint((*(end_-1)).position * coeffs.x() + coeffs.y(), (*(end_-1)).position));

		return LRCoef(coeffs.x(), coeffs.y());
	}

	int TimeSeries::PieceWise(double rate, DoubleTimeList* result)
	{
		DoubleTimeList cost;
		DoubleTimePoint p;

		if (Count() < 3)
			return 0;

		// copy this to result series
		result->assign(begin_, end_);

		// initialize the cost list
		p.position = (*begin_).position;
		p.value = DBL_MAX;
		cost.push_back(p);
		for (size_t i = 1; i < Count() - 1; i++)
		{
			double interpolateValue = TLUtils::LinearInterpolate(*(begin_ + i - 1), *(begin_ + i + 1), (*(begin_ + i)).position);
			p.value = std::fabs(interpolateValue - (*(begin_ + i)).value) / (interpolateValue + (*(begin_ + i)).value);
			p.position = (*(begin_ + i)).position;
			cost.push_back(p);
		}
		p.position = (*(end_-1)).position;
		p.value = DBL_MAX;
		cost.push_back(p);

		struct PointCompare {
			bool operator() (DoubleTimePoint p1, DoubleTimePoint p2) { return p1.value < p2.value; }
		} compareObj;

		int num = (int)(rate * Count() / 100);
		size_t previous_index;
		bool use_previous_index = false;
		int previous_hits = 0;
		while (num > 0)
		{
			size_t index;
			if (!use_previous_index)
			{
				auto cit = std::min_element(cost.begin(), cost.end(), compareObj);
				index = cit - cost.begin();
			}
			else
			{
				index = previous_index;
				previous_hits++;
			}

			time_t position = cost[index].position;
			double min_cost = cost[index].value;
			assert(index > 0);
			assert(index < cost.size() - 1);
			cost.erase(cost.begin() + index);
			result->erase(result->begin() + index);

			// update the cost for point: index - 1
			if (index > 1)
			{
				double interpolateValue = TLUtils::LinearInterpolate(result->at(index - 2), result->at(index), result->at(index - 1).position);
				cost[index - 1].value = std::fabs(interpolateValue - result->at(index - 1).value) / (interpolateValue + result->at(index - 1).value);
				assert(cost[index - 1].position == result->at(index - 1).position);
			}

			// update the cost for point: index
			if (index < cost.size() - 1)
			{
				double interpolateValue = TLUtils::LinearInterpolate(result->at(index - 1), result->at(index + 1), result->at(index).position);
				cost[index].value = std::fabs(interpolateValue - result->at(index).value) / (interpolateValue + result->at(index).value);
				assert(cost[index].position == result->at(index).position);
			}

			if (cost[index - 1].value < min_cost || cost[index].value < min_cost)
			{
				previous_index = cost[index - 1].value < cost[index].value ? index - 1 : index;
				use_previous_index = true;
			}
			else
			{
				use_previous_index = false;
			}

			num--;
		}
		std::cout << "num: " << rate * Count() / 100 << ", previous previous_hits: " << previous_hits << std::endl;

		return result->size();
	}
}