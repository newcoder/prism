#include <gtest/gtest.h>
#include "asset.h"
#include "store.h"
#include "util.h"
#include "indicator.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class AssetTest : public testing::Test
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

TEST_F(AssetTest, testLoad)
{
	std::string symbols;
	bool ret = store_->GetBlock("其它板块\\A股板块", symbols);
	EXPECT_TRUE(ret);

	std::vector<std::string> elems;
	kyotocabinet::strsplit(symbols, '\n', &elems);
	
	AssetsProvider loader(store_);
	int count = loader.LoadAssets(elems, 2011, 2013);
	std::cout << "symbols loaded: " << count << std::endl;
}

TEST_F(AssetTest, testLoadAll)
{
	AssetsProvider loader(store_);
	int count = loader.LoadAll();

	std::cout << "symbols loaded: " << count << std::endl;
	
}

TEST_F(AssetTest, testLoadPatterns)
{
	AssetsProvider loader(store_);
	// load symbols in the blocks and match the patterns...
	int count = loader.LoadAssets("patterns=SH600\\d{3},SZ001\\d{3};blocks=行业\\医药,行业\\计算机", 2010, 2012);

	std::cout << "symbols loaded: " << count << std::endl;

}

TEST_F(AssetTest, testScales)
{
	AssetsProvider loader(store_);

	std::string symbol = "SH600198";
	std::string another_symbol = "SZ002015";
	std::vector<std::string> elems;
	elems.push_back(symbol);
	elems.push_back(another_symbol);
	int count = loader.LoadAssets(elems, 2011, 2013);

	auto asset = loader.Get(symbol);
	EXPECT_TRUE(asset != nullptr);
	auto daily = asset->scales(DATA_TYPE_DAILY, 1);
	EXPECT_TRUE(daily != nullptr);

	auto two_days = asset->scales(DATA_TYPE_DAILY, 2);
	EXPECT_TRUE(two_days != nullptr);
	auto weekly = asset->scales(DATA_TYPE_WEEKLY, 1);
	EXPECT_TRUE(weekly != nullptr);
	auto two_weeks = asset->scales(DATA_TYPE_WEEKLY, 2);
	EXPECT_TRUE(two_weeks != nullptr);
	auto monthly = asset->scales(DATA_TYPE_MONTHLY, 1);
	EXPECT_TRUE(monthly != nullptr);
	auto two_months = asset->scales(DATA_TYPE_MONTHLY, 2);
	EXPECT_TRUE(two_months != nullptr);

	auto macd = daily->indicators("MACD_12_26_9");
	EXPECT_TRUE(macd != nullptr);
	auto ema = weekly->indicators("EMA_10");
	EXPECT_TRUE(ema != nullptr);
	macd = weekly->indicators("MACD_12_26_9");
	EXPECT_TRUE(macd != nullptr);
	auto cr = two_days->indicators("CR_20");
	EXPECT_TRUE(cr != nullptr);

	// try another symbol
	asset = loader.Get(another_symbol);
	EXPECT_TRUE(asset != nullptr);
	daily = asset->scales(DATA_TYPE_DAILY, 1);
	EXPECT_TRUE(daily != nullptr);
	ema = daily->indicators("EMA_30");
	EXPECT_TRUE(ema != nullptr);

}

TEST_F(AssetTest, testAssetIndexer)
{
	AssetsProvider loader(store_);

	std::string symbol = "SH600198";
	std::string another_symbol = "SZ002015";
	std::vector<std::string> elems;
	elems.push_back(symbol);
	elems.push_back(another_symbol);
	int count = loader.LoadAssets(elems, 2011, 2013);

	auto asset = loader.Get(symbol);
	AssetIndexer *asset_indexer = new AssetIndexer(asset);
	time_t day = StringToDate("2012-12-21", "%d-%d-%d");
	for (int i = 0; i < 33; i++)
	{
		day = day + 24 * 3600;		
		asset_indexer->ForwardTo(day);
		time_t date = asset_indexer->GetIndexTime();
		std::string date_str = TimeToString(date, "%Y-%m-%d");
		std::cout << date_str << std::endl;
	}

	// try multiple scales
	auto scale_indexer_two_days = asset_indexer->scale_indexers(DATA_TYPE_DAILY, 2);
	auto scale_indexer_weekly = asset_indexer->scale_indexers(DATA_TYPE_WEEKLY, 1);
	
	asset_indexer->ForwardTo(day);
	std::string date_str = TimeToString(day, "%Y-%m-%d");
	std::cout << "move to: " << date_str << std::endl;
	
	time_t date = scale_indexer_two_days->GetIndexTime();
	date_str = TimeToString(date, "%Y-%m-%d");
	std::cout << date_str << std::endl;

	date = scale_indexer_weekly->GetIndexTime();
	date_str = TimeToString(date, "%Y-%m-%d");
	std::cout << date_str << std::endl;

}