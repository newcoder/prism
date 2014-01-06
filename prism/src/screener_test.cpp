#include <gtest/gtest.h>
#include "asset.h"
#include "store.h"
#include "util.h"
#include "indicator.h"
#include "screener.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class ScreenerTest : public testing::Test
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


TEST_F(ScreenerTest, testScreen)
{
	AssetsProvider loader(store_);
	int count = loader.LoadAll();

	std::cout << "symbols loaded: " << count << std::endl;
	
	AssetIndexerList asset_indexer_list;
	auto assets = loader.assets();
	for (auto a : assets)
	{
		asset_indexer_list.push_back(AssetIndexer(a.second));
	}

	// move to the last position
	for (auto& ai : asset_indexer_list)
		ai.ToEnd();
	// screen by CR rule
	auto cr_rule = std::make_shared<CRRule>(20);
	auto screener = std::make_shared<Screener>(cr_rule);
	std::vector<int> result;
	screener->Screen(asset_indexer_list, &result);
	std::cout << "symbols screened by CR: " << result.size() << std::endl;
	for (auto i : result)
	{
		std::cout << TimeToString(asset_indexer_list[i].GetIndexTime(), "time: %Y-%m-%d, ") << asset_indexer_list[i].asset()->symbol() << std::endl;
	}

	// move to another day
	time_t day = StringToDate("2013-06-21", "%d-%d-%d");
	for (auto& ai : asset_indexer_list)
	{
		ai.ToBegin();
		ai.ForwardTo(day);
	}
	// screen by MACD
	auto macd_rule = std::make_shared<MACDRule>(12, 26, 9);
	macd_rule->set_data_type(DATA_TYPE_WEEKLY);
	macd_rule->set_data_num(1);
	screener = std::make_shared<Screener>(macd_rule);
	screener->Screen(asset_indexer_list, &result);
	std::cout << "symbols screened by MACD: " << result.size() << std::endl;
	for (auto i : result)
	{
		std::cout << TimeToString(asset_indexer_list[i].GetIndexTime(), "time: %Y-%m-%d, ") << asset_indexer_list[i].asset()->symbol() << std::endl;
	}
}