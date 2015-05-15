#include <gtest/gtest.h>
#include "strategy.h"
#include "strategy_screener.h"
#include "store.h"
#include "asset.h"
#include "time_series.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class StrategyTest : public testing::Test
{
public:
	std::shared_ptr<KCStore> store_;
	virtual void SetUp()
	{
		store_ = std::make_shared<KCStore>();
		bool ret = store_->Open(kDataPath + "TestData8.kch");
		EXPECT_TRUE(ret);
	}

	virtual void TearDown()
	{
		store_->Close();
	}
};

TEST_F(StrategyTest, testParse)
{
	prism::JsonDoc doc;
	bool ret = prism::ParseJson(kDataPath + "strategy.txt", doc);
	EXPECT_TRUE(ret);
	Strategy strategy("strategy.txt");
	ret = strategy.Parse(&doc);
	EXPECT_TRUE(ret);

	prism::JsonArchive ar(kDataPath + "strategy_test.txt");
	ret = ar.Open();
	EXPECT_TRUE(ret);
	prism::JsonSerializer* writer = ar.GetSerializer();
	strategy.Serialize(writer);
	ar.Close();
}

//TEST_F(StrategyTest, testRun)
//{
//	//StrategyRunner runner(store_);
//	//bool ret = runner.Load(kDataPath + "strategy.txt");
//	//EXPECT_TRUE(ret);
//
//	//StrategyObserver observer(&runner);
//	//runner.set_observer(&observer);
//	//runner.Initialize();
//	//runner.Run();
//
//	////TLUtils::Dump(kDataPath + "perf.csv", &observer.performance_series());
//	//std::vector<Transaction> trans = observer.transactions();
//	//for (size_t i = 0; i < trans.size(); i++)
//	//{
//	//	std::cout << trans[i].type_ << ", "
//	//	<< trans[i].asset_->symbol() << ", "
//	//	<< TimeToString(trans[i].time_, "%Y-%m-%d, ")
//	//	<< trans[i].price_ << ", "
//	//	<< trans[i].shares_<< std::endl;
//	//}
//
//}
//
//TEST_F(StrategyTest, testScreener)
//{
////	StrategyScreener screener(&store, kDataPath + "strategy.txt");
//	//			
//	//MACDStrategyScreener* macd_screener = new MACDStrategyScreener(store_, kDataPath + "strategy.txt", 12, 26, 9, true, 4, 0.000001);
//	//macd_screener->Test();
//	//macd_screener->DumpResult(kDataPath + "screen_perf.csv");
//	//delete macd_screener;
//}
//
//class MACDScreenerResult
//{
//public:
//	int short_period;
//	int long_period;
//	int signal_period;
//	bool linear_predict;
//	int look_back;
//	double threshold;
//	int positives;
//};
//
//TEST_F(StrategyTest, testMACDScreener)
//{
//	//std::vector<MACDScreenerResult> screen_results;
//
//	//for (int i = 11; i <= 13; i++)
//	//{// short
//	//	for (int j = 25; j <= 27; j++)
//	//	{// long
//	//		if (i >= j) continue;
//	//		for (int k = 8; k <= 10; k++)
//	//		{ // signal
//	//			MACDStrategyScreener* macd_screener = new MACDStrategyScreener(store_, kDataPath + "strategy.txt", i, j, k, false, 3, 0.000001);
//	//			macd_screener->Test();
//	//			int num_positive = macd_screener->GetPositiveNum();
//	//			
//	//			std::string file_name = std::string("Screener_MACD") + "_" + std::to_string(static_cast<long long>(i)) + "_" +
//	//				std::to_string(static_cast<long long>(j)) + "_" + 
//	//				std::to_string(static_cast<long long>(k)) + "_" +
//	//				std::to_string(static_cast<long long>(num_positive)) + ".csv";
//	//			std::cout << file_name << "\t\tdone!" << std::endl;
//	//			macd_screener->DumpResult(kDataPath + file_name);					
//	//			
//	//			MACDScreenerResult result;
//	//			result.linear_predict = true;
//	//			result.long_period = j;
//	//			result.short_period = i;
//	//			result.signal_period = k;
//	//			result.look_back = 4;
//	//			result.threshold = 0.000001;
//	//			result.positives = num_positive;
//	//			screen_results.push_back(result);
//
//	//			delete macd_screener;
//	//		}
//	//	}
//	//}
//
//	//	//struct perf_compare {
//	//	//	bool operator ()(const StrategyPerformance& a, const StrategyPerformance& b) const {
//	//	//		return a.profit_rate_ > b.profit_rate_;
//	//	//	}
//	//	//};
//	//	//std::sort(performance_.begin(), performance_.end(), perf_compare());
//
//	//std::ofstream of;
//	//of.open(kDataPath + "screen_results.csv", std::ios::out);
//	//size_t count = screen_results.size();
//	//for (size_t i = 0; i < count; ++i)
//	//{
//	//	MACDScreenerResult result = screen_results.at(i);
//	//	of << result.short_period << "," << result.long_period << "," << result.signal_period << ","
//	//		<< result.linear_predict << "," << result.look_back << ","
//	//		<< result.threshold << "," << result.positives << std::endl;
//	//}
//	//of.close();
//
//}