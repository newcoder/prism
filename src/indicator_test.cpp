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
	HLOCList hloc_list;
	DoubleTimeList tl;

	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
		ret = store.GetAll("SH600658", &hloc_list);
		EXPECT_TRUE(ret);
		EXPECT_TRUE(hloc_list.size() > 0);

		TLUtils::Populate(&hloc_list, PRICE_TYPE_C, &tl);
		EXPECT_TRUE(tl.size() > 0);
		EXPECT_EQ(tl.size(), hloc_list.size());
	}

	virtual void TearDown()
	{
		store.Close();
	}
};


TEST_F(TestIndicator, testSMA)
{
	auto sma = std::make_unique<SMA>(20);
	TimeSeries ts(tl.begin(), tl.end());
	ts.CalculateIndicator(sma.get());
	std::cout << "count after SMA: " << sma->result()->size() << std::endl;

	TLUtils::Dump(kDataPath + "SMA.csv", sma->result());
}

TEST_F(TestIndicator, testEMA)
{
	auto ema12 = std::make_unique<EMA>(12);
	auto ema26 = std::make_unique<EMA>(26);
	TimeSeries ts(tl.begin(), tl.end());
	ts.CalculateIndicator(ema12.get());
	ts.CalculateIndicator(ema26.get());

	std::cout << "count after EMA12: " << ema12->result()->size() << std::endl;
	std::cout << "count after EMA26: " << ema26->result()->size() << std::endl;

	TLUtils::Remove(ema12->result(), 14);

	TLUtils::Dump(kDataPath + "EMA12_EMA26.csv", ema12->result(), ema26->result());
}


TEST_F(TestIndicator, testMACD)
{
	auto macd = std::make_unique<MACD>(12, 26, 9);
	TimeSeries ts(tl.begin(), tl.end());
	ts.CalculateIndicator(macd.get());
	std::cout << "count after MACD: " << macd->macd()->size() << std::endl;

	std::cout << "count after MACD, signal: " << macd->signal()->size() << std::endl;
	TLUtils::Dump(kDataPath + "MACD-Signal.csv", macd->macd(), macd->signal());
	std::cout << "count after MACD, histogram: " << macd->histogram()->size() << std::endl;
	TLUtils::Dump(kDataPath + "MACD-Histogram.csv", macd->histogram());

	auto co = macd->cross_overs();
	std::cout << "count of crossovers: " << co->size() << std::endl;
	auto crossOverProfit = std::make_unique<DoubleTimeList>();
	for (size_t i = 0; i < co->size(); i++)
	{
		crossOverProfit->push_back(DoubleTimePoint(co->at(i).profit, co->at(i).time));
	}

	struct positive_counter {
		bool operator ()(const DoubleTimePoint& profit) const {
			return profit.value > 0;
		}
	};
	int count = std::count_if(crossOverProfit->begin(), crossOverProfit->end(), positive_counter());
	std::cout << "num of positive profit:" << count << std::endl;

	int win_length = 0, loss_length = 0;
	double win_area = 0.0F, loss_area = 0.0F;
	for (size_t i = 1; i < co->size() - 1; i++)
	{
		if (co->at(i + 1).profit > 0)
		{
			win_length += co->at(i).length;
			win_area += co->at(i).area;
		}
		else
		{
			loss_length += co->at(i).length;
			loss_area += co->at(i).area;
		}
	}
	
	std::cout << "total length of wins:" << win_length << std::endl;
	std::cout << "total length of loss:" << loss_length << std::endl;
	std::cout << "total area of wins:" << win_area << std::endl;
	std::cout << "total area of loss:" << loss_area << std::endl;

	TLUtils::Dump(kDataPath + "MACD-Crossover-Histogram.csv", macd->histogram(), crossOverProfit.get());
	TLUtils::Dump(kDataPath + "MACD-Profit.csv", crossOverProfit.get());
}

TEST_F(TestIndicator, testRSI)
{
	auto rsi = std::make_unique<RSI>(12);
	TimeSeries ts(tl.begin(), tl.end());

	ts.CalculateIndicator(rsi.get());
	std::cout << "count after rsi: " << rsi->result()->size() << std::endl;
	TLUtils::Dump(kDataPath + "RSI.csv", rsi->result());
}

TEST_F(TestIndicator, testKDJ)
{
	auto kdj = std::make_unique<KDJ>(9, 2, 2);
	HLOCSeries hs(hloc_list.begin(), hloc_list.end());
	hs.CalculateIndicator(kdj.get());

	std::cout << "K% count: " << kdj->k()->size() << std::endl;
	std::cout << "D% count: " << kdj->d()->size() << std::endl;
	std::cout << "J count: " << kdj->j()->size() << std::endl;

	TLUtils::Dump(kDataPath + "K_D_J.csv", kdj->k(), kdj->d(), kdj->j());

}

TEST_F(TestIndicator, testCR)
{
	auto cr = std::make_unique<CR>(20);
	HLOCSeries hs(hloc_list.begin(), hloc_list.end());
	hs.CalculateIndicator(cr.get());

	TLUtils::Dump(kDataPath + "CR.csv", cr->result());

}
