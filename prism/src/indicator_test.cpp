#include "time_series.h"
#include "store.h"
#include "indicator.h"
#include "hloc_series.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class TestIndicator : public testing::Test
{
public:
	KCStore store;
	int count;
	HLOCList *hloc_list;
	DoubleTimeList tl;

	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
		hloc_list = new prism::HLOCList();
		ret = store.GetAll("SH600658", hloc_list);
		EXPECT_TRUE(ret);
		EXPECT_TRUE(hloc_list->size() > 0);

		TLUtils::Populate(hloc_list, PRICE_TYPE_C, &tl);
		EXPECT_TRUE(tl.size() > 0);
		EXPECT_EQ(tl.size(), hloc_list->size());
	}

	virtual void TearDown()
	{
		store.Close();
		delete hloc_list;
	}
};


TEST_F(TestIndicator, testSMA)
{
	SMA* sma = new SMA(20);
	TimeSeries ts(tl.begin(), tl.end());
	ts.CalculateIndicator(sma);
	std::cout << "count after SMA: " << sma->result()->size() << std::endl;

	TLUtils::Dump(kDataPath + "SMA.csv", sma->result());
}

TEST_F(TestIndicator, testEMA)
{
	EMA* ema12 = new EMA(12);
	EMA* ema26 = new EMA(26);
	TimeSeries ts(tl.begin(), tl.end());
	ts.CalculateIndicator(ema12);
	ts.CalculateIndicator(ema26);

	std::cout << "count after EMA12: " << ema12->result()->size() << std::endl;
	std::cout << "count after EMA26: " << ema26->result()->size() << std::endl;

	TLUtils::Remove(ema12->result(), 14);

	TLUtils::Dump(kDataPath + "EMA12_EMA26.csv", ema12->result(), ema26->result());
}


TEST_F(TestIndicator, testMACD)
{
	MACD* macd = new MACD(12, 26, 9);
	TimeSeries ts(tl.begin(), tl.end());
	ts.CalculateIndicator(macd);
	std::cout << "count after MACD: " << macd->macd()->size() << std::endl;

	std::cout << "count after MACD, signal: " << macd->signal()->size() << std::endl;
	TLUtils::Dump(kDataPath + "MACD-Signal.csv", macd->macd(), macd->signal());
	std::cout << "count after MACD, histogram: " << macd->histogram()->size() << std::endl;
	TLUtils::Dump(kDataPath + "MACD-Histogram.csv", macd->histogram());

	CrossOverList* co = macd->cross_overs();
	std::cout << "count of crossovers: " << co->size() << std::endl;
	DoubleTimeList crossOverIntegral;
	for (size_t i = 0; i < co->size(); i++)
	{
		crossOverIntegral.push_back(DoubleTimePoint(co->at(i).integral, co->at(i).time));
	}
	TLUtils::Dump(kDataPath + "MACD-Crossover-Histogram.csv", macd->histogram(), &crossOverIntegral);
	TLUtils::Dump(kDataPath + "MACD-Crossover.csv", macd->macd(), macd->signal(), &crossOverIntegral);
}

TEST_F(TestIndicator, testRSI)
{
	RSI* rsi = new RSI(12);
	TimeSeries ts(tl.begin(), tl.end());

	ts.CalculateIndicator(rsi);
	std::cout << "count after rsi: " << rsi->result()->size() << std::endl;
	TLUtils::Dump(kDataPath + "RSI.csv", rsi->result());
}

TEST_F(TestIndicator, testKDJ)
{
	KDJ* kdj = new KDJ(9, 2, 2);
	HLOCSeries hs(hloc_list->begin(), hloc_list->end());
	hs.CalculateIndicator(kdj);

	std::cout << "K% count: " << kdj->k()->size() << std::endl;
	std::cout << "D% count: " << kdj->d()->size() << std::endl;
	std::cout << "J count: " << kdj->j()->size() << std::endl;

	TLUtils::Dump(kDataPath + "K_D_J.csv", kdj->k(), kdj->d(), kdj->j());

}
