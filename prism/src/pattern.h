// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// structure and classes for pattern.

#ifndef PATTERN_H
#define PATTERN_H

#include "common.h"
#include <vector>
#include <algorithm>
#include <set>
#include <map>

namespace prism
{

	class IPattern {
	public:
		virtual ~IPattern(){}
		virtual bool Match(const DoubleTimeList::const_iterator& current, const DoubleTimeList::const_iterator& end) const = 0;
		virtual size_t Size() const = 0;
		virtual void SetDelta(double fixed_delta, double adjacent_delta) = 0;
	};

	class Pattern;
	
	class PointsGroup 
	{
	public:
		PointsGroup(Pattern* owner, const std::set<int> points, bool adjacent = false) : owner_(owner), points_(points), adjacent_(adjacent) {}
		~PointsGroup() {}
		bool In(int point) const { return std::find(points_.begin(), points_.end(), point) != points_.end(); }
		size_t Size() const { return points_.size(); }
		bool IsAdjacentGroup() const { return adjacent_; }
		bool CheckAdjacence(DoubleList* list) const;
	private:
		double Delta() const;	
	private:
		Pattern* owner_;
		std::set<int> points_;
		bool adjacent_;
	};

	class Pattern : public IPattern
	{
	public:
		Pattern(double fixed_delta = kDefaultFixedDelta, double adjacent_delta = kDefaultAdjacentDelta) : fixed_delta_(fixed_delta), adjacent_delta_(adjacent_delta) {}
	public:
		bool Match(const DoubleTimeList::const_iterator& current, const DoubleTimeList::const_iterator& end) const;	
		void Generate(const DoubleTimeList::const_iterator& begin, const DoubleTimeList::const_iterator& end);		
		size_t Size() const;
		void SetDelta(double fixed_delta, double adjacent_delta) { fixed_delta_ = fixed_delta; adjacent_delta_ = adjacent_delta; }
		void AddGroup(const PointsGroup& group) { groups_.push_back(group); }
		double GetAdjacentDelta() const { return adjacent_delta_; }

	private:
		double fixed_delta_;
		double adjacent_delta_;
		std::vector<PointsGroup> groups_;
	};

	class PairPattern : public IPattern
	{
	public:
		PairPattern(IPattern* first, IPattern* second) : first_(first), second_(second) {}
	public:
		bool Match(const DoubleTimeList::const_iterator& current, const DoubleTimeList::const_iterator& end) const;
		size_t Size() const { return first_->Size() + second_->Size() - 2; };
		void SetDelta(double fixed_delta, double adjacent_delta) { first_->SetDelta(fixed_delta, adjacent_delta); second_->SetDelta(fixed_delta, adjacent_delta); }
	private:
		IPattern* first_;
		IPattern* second_;
	};

	// classic patterns
	typedef enum {
		PATTERN_DOUBLE_TOPS = 1,
		PATTERN_DOUBLE_BOTTOMS,
		PATTERN_FIVE_WAVES_UP,
		PATTERN_THREE_WAVES_DOWN,
		PATTERN_TRIPLE_TOPS,
		PATTERN_TRIPLE_BOTTOMS,
		PATTERN_HEAD_SHOULDERS_TOP,
		PATTERN_HEAD_SHOULDERS_BOTTOM
	} PATTERN_TYPE;

	const int kFreeGroup = -1;
	const int kAdjacentGroup = -2;

	static const int DOUBLE_TOPS[] = { kFreeGroup, 0, 4, kFreeGroup, 2, kAdjacentGroup, 1, 3 };
	static const int DOUBLE_BOTTOMS[] = { kAdjacentGroup, 1, 3, kFreeGroup, 2, kFreeGroup, 0, 4 };
	static const int FIVE_WAVES_UP[] = { kFreeGroup, 0, kFreeGroup, 2, kFreeGroup, 1, kFreeGroup, 4, kFreeGroup, 3, kFreeGroup, 5};


	class PatternFactory
	{
	public:
		static std::shared_ptr<Pattern> GetPattern(PATTERN_TYPE type);
	private:
		static std::shared_ptr<Pattern> CreatePattern(PATTERN_TYPE type, const int *arr, int size);
	private:
		static std::map<PATTERN_TYPE, std::shared_ptr<Pattern>> sPatterns;
	};
}

#endif