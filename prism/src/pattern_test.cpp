#include "time_series.h"
#include "pattern.h"
#include "store.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";


class TestPattern : public testing::Test
{
public:
	KCStore store;
	int count;
	DoubleTimeList tl, result;

	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
		EXPECT_TRUE(ret);

		HLOCList *hloc_list = new prism::HLOCList();
		ret = store.GetAll("SH601588", hloc_list);
		EXPECT_TRUE(ret);

		TLUtils::Populate(hloc_list, PRICE_TYPE_H, &tl);
		EXPECT_EQ(tl.size(), hloc_list->size());

		TimeSeries ts(tl.begin(), tl.end());
		ts.Normalize();
		ts.FindLocalExtremas(0.1, &result);
	}

	virtual void TearDown()
	{
		store.Close();
	}
};

TEST_F(TestPattern, testMatch)
{
	DoubleTimeList matches;
	IPattern* pattern = PatternFactory::GetPattern(PATTERN_DOUBLE_BOTTOMS);

	TimeSeries ts(tl.begin(), tl.end());
	ts.FindLocalExtremas(0.03, &result);

	pattern->SetDelta(0.0, 0.04);
				
	TimeSeries tsExtrema(result.begin(), result.end());
	int count = tsExtrema.MatchPattern(pattern, &matches);
	std::cout <<"matches: " << count << std::endl;
	if (count > 0)
	{
		TLUtils::Dump(kDataPath + "10_0_4_double_bottoms.csv", &matches);
	}

}

TEST_F(TestPattern, testGeneratePatternAndMatch)
{
	DoubleTimeList matches;
	TimeSeries ts(tl.begin(), tl.end());
	ts.FindLocalExtremas(0.03, &result);	
	
	TLUtils::Dump(kDataPath + "dump_for_match.csv", &result);

	// match the pattern formed by last points
	TimeSeries tsExtrema(result.begin(), result.end());
	IPattern* pattern = tsExtrema.ExtractPattern(result.size() - 4, result.size() - 1, 0.02, 0.02);
	int count = tsExtrema.MatchPattern(pattern, &matches);
	std::cout << "matches: " << count << std::endl;
	if (count > 0)
	{
		TLUtils::Dump(kDataPath + "10_0_4_generate_pattern.csv", &matches);
	}

}