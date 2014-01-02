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
	std::shared_ptr<HLOCList> hl;

	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
		hl = std::make_shared<HLOCList>();
		ret = store.GetAll("SH600648", hl);	
	}

	virtual void TearDown()
	{
		store.Close();
	}
};

TEST_F(TestHLOCSeries, testNormalize)
{
	HLOCSeries hs(hl->begin(), hl->end());
	hs.Normalize();
	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hl, PRICE_TYPE_H, tl);
	std::cout << "count after normalize: " << tl->size() << std::endl;

	TLUtils::Dump(kDataPath + "normalized.csv", tl);
}

TEST_F(TestHLOCSeries, testRestoreSplit)
{
	HLOCSeries hs(hl->begin(), hl->end());

	SplitOptions so;
	so.bonusShares = 0.2;
	so.dividend = 0.5;
	so.rationedPrice = 4.56;
	so.rationedShares = 1;
	so.time = hl->at(30).time;

	hs.RestoreSplit(so, true);
	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hl, PRICE_TYPE_H, tl);

	std::cout << "count after restore split: " << tl->size() << std::endl;
	TLUtils::Dump(kDataPath + "split_restored.csv", tl);
}

TEST_F(TestHLOCSeries, testShrinkByNum)
{
	HLOCSeries hs(hl->begin(), hl->end());

	auto hlFiveDays(std::make_shared<HLOCList>());
	hs.ShrinkByNum(5, hlFiveDays);
	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hlFiveDays, PRICE_TYPE_H, tl);

	std::cout << "count after shrink: " << tl->size() << std::endl;
	TLUtils::Dump(kDataPath + "shrink_5days.csv", tl);
}

TEST_F(TestHLOCSeries, testShrinkByWeek)
{
	HLOCSeries hs(hl->begin(), hl->end());

	auto hlWeekly(std::make_shared<HLOCList>());
	hs.ShrinkByWeek(hlWeekly);
	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hlWeekly, PRICE_TYPE_H, tl);

	std::cout << "count after shrink: " << tl->size() << std::endl;
	TLUtils::Dump(kDataPath + "shrink_weekly.csv", tl);
}

TEST_F(TestHLOCSeries, testShrinkByMonth)
{
	HLOCSeries hs(hl->begin(), hl->end());

	auto hlMonthly(std::make_shared<HLOCList>());
	hs.ShrinkByMonth(hlMonthly);
	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hlMonthly, PRICE_TYPE_H, tl);

	std::cout << "count after shrink: " << tl->size() << std::endl;
	TLUtils::Dump(kDataPath + "shrink_monthly.csv", tl);
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