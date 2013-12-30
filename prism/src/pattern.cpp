// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// structure and classes for pattern.

#include "pattern.h"
#include <algorithm>

namespace prism
{
	double PointsGroup::Delta() const
	{ 
		return owner_->GetAdjacentDelta(); 
	}

	bool PointsGroup::CheckAdjacence(DoubleList* list) const
	{
		auto cit = list->begin();
		if (list->size() < 2) return true;
		double max = -DBL_MAX, min = DBL_MAX;
		while (cit != list->end())
		{
			if(max < *cit)
				max = *cit;
			if(min > *cit)
				min = *cit;
			cit++;
		}
		double delta = std::fabs((max - min)/(max + min));
		
		return delta < Delta();
	}

	size_t Pattern::Size() const
	{
		auto gcit = groups_.begin();
		size_t sum = 0;
		while (gcit != groups_.end())
		{
			sum += (*gcit).Size();
			gcit++;
		}
		return sum;
	}

	void Pattern::Generate(const DoubleTimeList::const_iterator& begin, const DoubleTimeList::const_iterator& end)
	{
		DoubleTimeList points;
		auto cit = begin;
		size_t i = 0;
		
		if (end - begin < 2)
			return;

		while (cit != end + 1)
		{
			points.push_back(DoubleTimePoint((*cit).value, i++));
			cit++;
		}
		// sort the points by value
		struct PointCompare {
			bool operator() (DoubleTimePoint p1, DoubleTimePoint p2) { return p1.value < p2.value; }
		} compareObj;

		std::sort(points.begin(), points.end(), compareObj);

		groups_.clear();
		std::set<int> pointsSet;
		bool adjacent;
		cit = points.begin();
		double previous;
		while (cit != points.end())
		{
			if (pointsSet.empty())
			{
				previous = (*cit).value;
				pointsSet.insert((int)(*cit).position);
			}
			else
			{
				double current = (*cit).value;
				double delta = std::fabs((current - previous) / (current + previous));
				if (delta < adjacent_delta_)
				{
					// adjacent point		
					pointsSet.insert((int)(*cit).position);
				}
				else
				{
					adjacent = pointsSet.size() > 1;
					AddGroup(PointsGroup(this, pointsSet, adjacent));
					pointsSet.clear();
					continue;
				}
			}
			cit++;
		}
		if (!pointsSet.empty())
		{
			adjacent = pointsSet.size() > 1;
			AddGroup(PointsGroup(this, pointsSet, adjacent));
		}

	}

	bool Pattern::Match(const DoubleTimeList::const_iterator& current, const DoubleTimeList::const_iterator& end) const
	{
		DoubleTimeList points;
		size_t positionCount = Size();
		auto cit = current;
		size_t i = 0;
		while(i < positionCount && cit != end)
		{
			points.push_back(DoubleTimePoint((*cit).value, i++));
			cit++;
		}
		if (i < positionCount)
		{
			// points not enough for matching
			return false;
		}

		// sort the points by value
		struct PointCompare {
			bool operator() (DoubleTimePoint p1,DoubleTimePoint p2) { return p1.value < p2.value; }
		} compareObj;

		std::sort(points.begin(), points.end(), compareObj);

		// check each groups
		auto gcit = groups_.begin();
		auto pcit = points.begin();
		double max_in_last_group = -DBL_MAX;

		while(gcit != groups_.end() && pcit != points.end())
		{
			PointsGroup group = *gcit;
			DoubleList values;		
			// check fixed delta
			double delta = std::fabs(((*pcit).value - max_in_last_group) / ((*pcit).value + max_in_last_group));
			if (delta < fixed_delta_)
				return false;

			// make sure points in the group
			size_t i = 0; 
			size_t group_size = group.Size();
			while (i < group_size && pcit != points.end())
			{
				if (!group.In((int)(*pcit).position))
					return false;				
				values.push_back((*pcit).value);
				pcit++;
				i++;
			}

			// check adjacent for AdjacentGroup
			if (group.IsAdjacentGroup())
			{
				if (!group.CheckAdjacence(&values))
					return false;
			}

			max_in_last_group = values[values.size() - 1];
			gcit++;
		}
		return true;
	}

	bool PairPattern::Match(const DoubleTimeList::const_iterator& current, const DoubleTimeList::const_iterator& end) const
	{
		return first_->Match(current, end) && second_->Match(current + first_->Size() - 2, end);
	}

	std::map<PATTERN_TYPE, Pattern*> PatternFactory::sPatterns;

	#define CREATE_PATTERN(type) \
		CreatePattern(PATTERN_##type, type, sizeof(type) / sizeof(int));

	Pattern* PatternFactory::GetPattern(PATTERN_TYPE type)
	{
		auto cit = sPatterns.find(type);
		if (cit != sPatterns.end())
		{
			return (*cit).second;
		}
		else
		{
			switch (type)
			{
			case PATTERN_DOUBLE_TOPS:
				return CREATE_PATTERN(DOUBLE_TOPS);
			case PATTERN_DOUBLE_BOTTOMS:
				return CREATE_PATTERN(DOUBLE_BOTTOMS);
			case PATTERN_FIVE_WAVES_UP:
				return CREATE_PATTERN(FIVE_WAVES_UP);
			default:
				break;
			}
		}
		return nullptr;
	}

	Pattern* PatternFactory::CreatePattern(PATTERN_TYPE type, const int *arr, int size)
	{
		Pattern* pattern = new Pattern();
		const int *p = arr;
		bool group_start = false;
		std::set<int> points_set;
		bool adjacent = false;
		for (int i = 0; i < size; ++i)
		{
			if (*p < 0)
			{
				// start of a group
				if (group_start)
				{
					pattern->AddGroup(PointsGroup(pattern, points_set, adjacent));
				}
				group_start = true;
				points_set.clear();
				adjacent = false;
			}
			if (*p == kAdjacentGroup)
			{
				adjacent = true;
			}
			if (*p >= 0)
			{
				points_set.insert(*p);
			}
			p++;
		}
		if (points_set.size() > 0)
		{
			pattern->AddGroup(PointsGroup(pattern, points_set, adjacent));
		}

		sPatterns.insert(std::make_pair(type, pattern));
		return pattern;
	}
}