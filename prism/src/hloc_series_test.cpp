#include "hloc_series.h"
#include "store.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class TestHLOCSeries : public testing::Test
{
public:
	KCStore store;
	int count;
	HLOCList hl;

	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
		ret = store.GetAll("SH600648", &hl);	
	}

	virtual void TearDown()
	{
		store.Close();
	}
};

TEST_F(TestHLOCSeries, testNormalize)
{
	HLOCSeries hs(hl.begin(), hl.end());
	hs.Normalize();
	DoubleTimeList tl;
	TLUtils::Populate(&hl, PRICE_TYPE_H, &tl);
	std::cout << "count after normalize: " << tl.size() << std::endl;

	TLUtils::Dump(kDataPath + "normalized.csv", &tl);
}

TEST_F(TestHLOCSeries, testRestoreSplit)
{
	HLOCSeries hs(hl.begin(), hl.end());

	SplitOptions so;
	so.bonusShares = 0.2;
	so.dividend = 0.5;
	so.rationedPrice = 4.56;
	so.rationedShares = 1;
	so.time = hl.at(30).time;

	hs.RestoreSplit(so, true);
	DoubleTimeList tl;
	TLUtils::Populate(&hl, PRICE_TYPE_H, &tl);

	std::cout << "count after restore split: " << tl.size() << std::endl;
	TLUtils::Dump(kDataPath + "split_restored.csv", &tl);
}

TEST_F(TestHLOCSeries, testShrinkByNum)
{
	HLOCSeries hs(hl.begin(), hl.end());

	HLOCList hlFiveDays;
	hs.ShrinkByNum(5, &hlFiveDays);
	DoubleTimeList tl;
	TLUtils::Populate(&hlFiveDays, PRICE_TYPE_H, &tl);

	std::cout << "count after shrink: " << tl.size() << std::endl;
	TLUtils::Dump(kDataPath + "shrink_5days.csv", &tl);
}

TEST_F(TestHLOCSeries, testShrinkByWeek)
{
	HLOCSeries hs(hl.begin(), hl.end());

	HLOCList hlWeekly;
	hs.ShrinkByWeek(&hlWeekly);
	DoubleTimeList tl;
	TLUtils::Populate(&hlWeekly, PRICE_TYPE_H, &tl);

	std::cout << "count after shrink: " << tl.size() << std::endl;
	TLUtils::Dump(kDataPath + "shrink_weekly.csv", &tl);
}

TEST_F(TestHLOCSeries, testShrinkByMonth)
{
	HLOCSeries hs(hl.begin(), hl.end());

	HLOCList hlMonthly;
	hs.ShrinkByMonth(&hlMonthly);
	DoubleTimeList tl;
	TLUtils::Populate(&hlMonthly, PRICE_TYPE_H, &tl);

	std::cout << "count after shrink: " << tl.size() << std::endl;
	TLUtils::Dump(kDataPath + "shrink_monthly.csv", &tl);
}

//TEST_F(TestHLOCSeries, testGetRSV)
//{
//	HLOCSeries hs(hl.begin(), hl.end());
//
//	DoubleTimeList tlRSV;
//	hs.GetRSV(9, &tlRSV);
//
//	std::cout << "count in RSV: " << tlRSV.size() << std::endl;
//	TLUtils::Dump(kDataPath + "rsv.csv", &tlRSV);
//}