#include "time_series.h"
#include "store.h"
#include "indicator.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class TestTimeSeries : public testing::Test
{
public:
	KCStore store;
	int count;

	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
	}

	virtual void TearDown()
	{
		store.Close();
	}
};

TEST_F(TestTimeSeries, testNormalize)
{
	DoubleTimeList tl;
	tl.push_back(DoubleTimePoint(23.2, 0));
	tl.push_back(DoubleTimePoint(43.2, 0));
	tl.push_back(DoubleTimePoint(53.12, 0));
	tl.push_back(DoubleTimePoint(73.22, 0));

	TimeSeries ts(tl.begin(), tl.end());
	ts.Normalize();

	for (size_t i=0; i<tl.size(); ++i)
		std::cout << tl[i].value << "\n";
}

TEST_F(TestTimeSeries, testDistance)
{
	auto tl = std::make_shared<DoubleTimeList>();
	auto tl1 = std::make_shared<DoubleTimeList>();
	auto tl2 = std::make_shared<DoubleTimeList>();

	tl->push_back(DoubleTimePoint(23.2, 1));
	tl->push_back(DoubleTimePoint(43.2, 2));
	tl->push_back(DoubleTimePoint(53.12, 8));
	tl->push_back(DoubleTimePoint(73.22, 10));
	tl->push_back(DoubleTimePoint(34.22, 12));

	tl1->push_back(DoubleTimePoint(13.2, 1));
	tl1->push_back(DoubleTimePoint(34.2, 2));
	tl1->push_back(DoubleTimePoint(73.22, 4));
	tl1->push_back(DoubleTimePoint(76.22, 5));
	tl1->push_back(DoubleTimePoint(13.2, 6));
	tl1->push_back(DoubleTimePoint(34.2, 7));
	tl1->push_back(DoubleTimePoint(73.22, 9));
	tl1->push_back(DoubleTimePoint(76.22, 12));

	tl2->push_back(DoubleTimePoint(10.2, 1));
	tl2->push_back(DoubleTimePoint(35.2, 4));
	tl2->push_back(DoubleTimePoint(89.12, 8));
	tl2->push_back(DoubleTimePoint(22.22, 9));
	tl2->push_back(DoubleTimePoint(34.22, 12));

	double d = TLUtils::Distance(tl, tl1);
	double d1 = TLUtils::Distance(tl1, tl);
	EXPECT_DOUBLE_EQ(d1, d);

	double d2 = TLUtils::Distance(tl1, tl2);
	double d3 = TLUtils::Distance(tl, tl2);
	EXPECT_GT(d1 + d2, d3);
	EXPECT_GT(d1 + d3, d2);	
	EXPECT_GT(d2 + d3, d1);

	std::cout << "d1:" << d1 << std::endl;
	std::cout << "d2:" << d2 << std::endl;
	std::cout << "d3:" << d3 << std::endl;
}

TEST_F(TestTimeSeries, testLocalExtrema)
{
	auto tl = std::make_shared<DoubleTimeList>();
	auto result = std::make_shared<DoubleTimeList>();

	tl->push_back(DoubleTimePoint(3.856, 1));
	tl->push_back(DoubleTimePoint(3.786, 2));
	tl->push_back(DoubleTimePoint(3.75, 3));
	tl->push_back(DoubleTimePoint(3.739, 4));
	tl->push_back(DoubleTimePoint(3.809, 5));
	tl->push_back(DoubleTimePoint(3.88, 6));
	tl->push_back(DoubleTimePoint(4.268, 7));
	tl->push_back(DoubleTimePoint(3.986, 8));
	tl->push_back(DoubleTimePoint(3.75, 9));
	tl->push_back(DoubleTimePoint(3.927, 10));
	tl->push_back(DoubleTimePoint(3.909, 11));
	tl->push_back(DoubleTimePoint(3.88, 12));
	tl->push_back(DoubleTimePoint(4.162, 13));
	tl->push_back(DoubleTimePoint(3.956, 14));
	tl->push_back(DoubleTimePoint(3.986, 15));
	tl->push_back(DoubleTimePoint(4.015, 16));
	tl->push_back(DoubleTimePoint(3.903, 17));
	tl->push_back(DoubleTimePoint(3.78, 18));
	tl->push_back(DoubleTimePoint(3.762, 19));
	tl->push_back(DoubleTimePoint(3.809, 20));
	tl->push_back(DoubleTimePoint(4.045, 21));
	tl->push_back(DoubleTimePoint(4.156, 22));
	tl->push_back(DoubleTimePoint(4.168, 23));
	tl->push_back(DoubleTimePoint(4.556, 24));
	tl->push_back(DoubleTimePoint(4.409, 25));
	tl->push_back(DoubleTimePoint(4.327, 26));
	tl->push_back(DoubleTimePoint(4.333, 27));

	TimeSeries ts(tl->begin(), tl->end());
	int ret = ts.FindLocalExtremas(0.03, result);
	for (size_t i=0; i<result->size(); ++i)
		std::cout << result->at(i).value << "\n";

	TLUtils::Dump(kDataPath + "check_localextrema_fixed.csv", tl, result);
}

TEST_F(TestTimeSeries, testPopulateAndDump)
{
	std::shared_ptr<HLOCList> hloc_list = std::make_shared<prism::HLOCList>();

	bool ret = store.GetAll("SH600668", hloc_list);
	EXPECT_TRUE(ret);

	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hloc_list, PRICE_TYPE_H, tl);
	EXPECT_EQ(tl->size(), hloc_list->size());
	
	TimeSeries ts(tl->begin(), tl->end());
	ts.Normalize();
	TLUtils::Dump(kDataPath + "dump_normalized.csv", tl);

	auto result = std::make_shared<DoubleTimeList>();
	ts.FindLocalExtremas(0.03, result);
	TLUtils::Dump(kDataPath + "dump_localExtrema.csv", result);

	// dump two series together
	TLUtils::Dump(kDataPath + "dump_OriginalAndExtrema.csv", tl, result);
}

TEST_F(TestTimeSeries, testLinearRegression)
{
	std::shared_ptr<HLOCList> hloc_list = std::make_shared<prism::HLOCList>();
	bool ret = store.GetAll("SH600678", hloc_list);
	EXPECT_TRUE(ret);

	auto tl = std::make_shared<DoubleTimeList>(); 
	auto result = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hloc_list->begin(), hloc_list->begin() + 100, PRICE_TYPE_H, tl);
	TimeSeries ts(tl->begin(), tl->end());
	LRCoef coef = ts.LinearRegression(result);
	std::cout << "A = " << coef.A << ", B = " << coef.B << std::endl;

	// dump two series together
	TLUtils::Dump(kDataPath + "dump_linear_regression.csv", tl, result);
}

TEST_F(TestTimeSeries, testLinearFit)
{
	auto tl = std::make_shared<DoubleTimeList>(); 
	auto result = std::make_shared<DoubleTimeList>();
	tl->push_back(DoubleTimePoint(10, 1));
	tl->push_back(DoubleTimePoint(5, 2));
	TimeSeries ts(tl->begin(), tl->end());
	LRCoef coef = ts.LinearRegression(result);
	double y = coef.A * 3 + coef.B;
	std::cout << "A = " << coef.A << ", B = " << coef.B << std::endl;

}

TEST_F(TestTimeSeries, testFindLocalExtremas)
{
	std::shared_ptr<HLOCList> hloc_list = std::make_shared<prism::HLOCList>();
	bool ret = store.GetAll("SH600287", hloc_list);
	EXPECT_TRUE(ret);

	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hloc_list, PRICE_TYPE_H, tl);
	EXPECT_EQ(tl->size(), hloc_list->size());

	TimeSeries ts(tl->begin(), tl->end());
	ts.Normalize();

	auto result = std::make_shared<DoubleTimeList>();
	ts.FindLocalExtremas(0.03, result);

	double dist = TLUtils::Distance(tl, result);
	std::cout << "dist: " << dist << std::endl;

	// make sure they have same begin position and end position
	EXPECT_EQ(result->at(0).position, tl->at(0).position);
	EXPECT_EQ(result->at(result->size() - 1).position, tl->at(tl->size() - 1).position);
}

TEST_F(TestTimeSeries, testFindTurningPoints)
{
	std::shared_ptr<HLOCList> hloc_list = std::make_shared<prism::HLOCList>();
	bool ret = store.GetAll("SH600177", hloc_list);
	EXPECT_TRUE(ret);

	auto tl = std::make_shared<DoubleTimeList>();;
	TLUtils::Populate(hloc_list, PRICE_TYPE_H, tl);
	EXPECT_EQ(tl->size(), hloc_list->size());

	TimeSeries ts(tl->begin(), tl->end());
	ts.Normalize();

	// find local min/max
	auto result = std::make_shared<DoubleTimeList>();
	ts.FindLocalExtremas(0.03, result);
	std::cout << "local extremas count: " << result->size() << std::endl;
	double dist = TLUtils::Distance(tl, result);
	std::cout << "dist, extremas vs original: " << dist << std::endl;

	// find turning points in the local min/max series
	auto turning_points = std::make_shared<DoubleTimeList>();;
	TimeSeries ts2(result->begin(), result->end());
	count = ts2.FindTurningPoints(0.05, 24*3600*8, turning_points);
	std::cout << "turning points count: " << count << std::endl;

	// dump two series together
	TLUtils::Dump(kDataPath + "dump_findTurningPoints.csv", result, turning_points);

	for (int i = 4; i < 20; i++) // percentage step
	{
		for (int j = 2; j < 20; j++) // time step
		{
			count = ts2.FindTurningPoints(0.01*i, 24*3600*j, turning_points);
			dist = TLUtils::Distance(result, turning_points); // distance between original series and turning points
			std::cout << "dp: " << 0.01*i << ", dt: " << j << ", count: " << count << ", dist: " << dist << std::endl;
			TLUtils::Dump(kDataPath + std::to_string(i) + "_" + std::to_string(j) + "_" + "dump_findTurningPoints.csv", result, turning_points);
		}
	}

	// make sure they have same begin position and end position
	EXPECT_EQ(result->at(0).position, turning_points->at(0).position);
	EXPECT_EQ(result->at(result->size() - 1).position, turning_points->at(turning_points->size() - 1).position);
}

TEST_F(TestTimeSeries, testThreshold)
{
	std::shared_ptr<HLOCList> hloc_list = std::make_shared<prism::HLOCList>();
	bool ret = store.GetAll("SH600328", hloc_list);
	EXPECT_TRUE(ret);

	auto tl = std::make_shared<DoubleTimeList>();
	TLUtils::Populate(hloc_list, PRICE_TYPE_H, tl);
	EXPECT_EQ(tl->size(), hloc_list->size());

	TimeSeries ts(tl->begin(), tl->end());
	ts.Normalize();

	std::cout << "original size: " << tl->size() << std::endl;
	double step = 0.01;
	auto result = std::make_shared<DoubleTimeList>();

	for (int i = 0; i < 20; i++)
	{
		ts.FindLocalExtremas(step * (i + 1), result);	
		TLUtils::Dump(kDataPath + std::to_string(i)+ "_dump_OriginalAndExtrema.csv", tl, result);
		double dist = TLUtils::Distance(tl, result);
		std::cout << "threshold: " << step * (i + 1) << "  dist: " << dist << "  size: " << result->size() << std::endl;
	}
}

TEST_F(TestTimeSeries, testPieceWise)
{
	std::shared_ptr<HLOCList> hloc_list = std::make_shared<prism::HLOCList>();
	bool ret = store.GetAll("SH601558", hloc_list);
	EXPECT_TRUE(ret);

	auto tl = std::make_shared<DoubleTimeList>();;
	TLUtils::Populate(hloc_list, PRICE_TYPE_H, tl);
	EXPECT_EQ(tl->size(), hloc_list->size());

	TimeSeries ts(tl->begin(), tl->end());
	ts.Normalize();
	
	auto result = std::make_shared<DoubleTimeList>();
	ts.PieceWise(89.5, result);
	
	std::cout << "count: " << result->size() << std::endl;

	double dist = TLUtils::Distance(tl, result);
	std::cout << "dist: " << dist << std::endl;

	// make sure they have same begin position and end position
	EXPECT_EQ(result->at(0).position, tl->at(0).position);
	EXPECT_EQ(result->at(result->size() - 1).position, tl->at(tl->size() - 1).position);

	// dump two series together
	TLUtils::Dump(kDataPath + "dump_piece_wise.csv", tl, result);

}

