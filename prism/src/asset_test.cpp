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
	KCStore store;
	virtual void SetUp()
	{
		bool ret = store.Open(kDataPath + "TestData8.kch");
		EXPECT_TRUE(ret);
	}

	virtual void TearDown()
	{
		store.Close();
	}
};

TEST_F(AssetTest, testLoad)
{
	std::string symbols;
	bool ret = store.GetBlock("ÆäËü°å¿é\\A¹É°å¿é", symbols);
	EXPECT_TRUE(ret);

	std::vector<std::string> elems;
	kyotocabinet::strsplit(symbols, '\n', &elems);
	
	AssetsProvider loader((IStore*)&store);
	int count = loader.LoadAssets(elems, 2011, 2013);
	std::cout << "symbols loaded: " << count << std::endl;
}

TEST_F(AssetTest, testLoadAll)
{
	AssetsProvider loader((IStore*)&store);
	bool ret = loader.LoadAll();
	EXPECT_TRUE(ret);

	std::cout << "symbols loaded: " << loader.assets()->size() << std::endl;
	
}

TEST_F(AssetTest, testScales)
{
	AssetsProvider loader((IStore*)&store);

	std::string symbol = "SH600196";
	std::string another_symbol = "SZ002010";
	std::vector<std::string> elems;
	elems.push_back(symbol);
	elems.push_back(another_symbol);
	int count = loader.LoadAssets(elems, 2011, 2013);

	Asset* asset = loader.Get(symbol);
	EXPECT_TRUE(asset != nullptr);
	AssetScale* daily = asset->scales(DATA_TYPE_DAILY, 1);
	EXPECT_TRUE(daily != nullptr);

	AssetScale* two_days = asset->scales(DATA_TYPE_DAILY, 2);
	EXPECT_TRUE(two_days != nullptr);
	AssetScale* weekly = asset->scales(DATA_TYPE_WEEKLY, 1);
	EXPECT_TRUE(weekly != nullptr);
	AssetScale* two_weeks = asset->scales(DATA_TYPE_WEEKLY, 2);
	EXPECT_TRUE(two_weeks != nullptr);
	AssetScale* monthly = asset->scales(DATA_TYPE_MONTHLY, 1);
	EXPECT_TRUE(monthly != nullptr);
	AssetScale* two_months = asset->scales(DATA_TYPE_MONTHLY, 2);
	EXPECT_TRUE(two_months != nullptr);

	MACD* macd = (MACD*)daily->indicators("MACD_12_26_9");
	EXPECT_TRUE(macd != nullptr);
	EMA* ema = (EMA*)weekly->indicators("EMA_10");
	EXPECT_TRUE(ema != nullptr);
	CR* cr = (CR*)two_days->indicators("CR_20");
	EXPECT_TRUE(cr != nullptr);

	// try another symbol
	asset = loader.Get(another_symbol);
	EXPECT_TRUE(asset != nullptr);
	daily = asset->scales(DATA_TYPE_DAILY, 1);
	EXPECT_TRUE(daily != nullptr);
	ema = (EMA*)daily->indicators("EMA_30");
	EXPECT_TRUE(ema != nullptr);

}

TEST_F(AssetTest, testAssetIndexer)
{
	AssetsProvider loader((IStore*)&store);

	std::string symbol = "SH600196";
	std::string another_symbol = "SZ002010";
	std::vector<std::string> elems;
	elems.push_back(symbol);
	elems.push_back(another_symbol);
	int count = loader.LoadAssets(elems, 2011, 2013);

	Asset* asset = loader.Get(symbol);
	AssetIndexer *asset_indexer = new AssetIndexer(asset);
	time_t day = StringToDate("2012-12-21", "%d-%d-%d");
	for (int i = 0; i < 33; i++)
	{
		day = day + 24 * 3600;		
		asset_indexer->MoveTo(day);		
		time_t date = asset_indexer->GetIndexTime();
		std::string date_str = TimeToString(date, "%Y-%m-%d");
		std::cout << date_str << std::endl;
	}

	// try multiple scales
	AssetScaleIndexer* scale_indexer_two_days = asset_indexer->scale_indexers(DATA_TYPE_DAILY, 2);
	AssetScaleIndexer* scale_indexer_weekly = asset_indexer->scale_indexers(DATA_TYPE_WEEKLY, 1);
	
	asset_indexer->MoveTo(day);
	std::string date_str = TimeToString(day, "%Y-%m-%d");
	std::cout << "move to: " << date_str << std::endl;
	
	time_t date = scale_indexer_two_days->GetIndexTime();
	date_str = TimeToString(date, "%Y-%m-%d");
	std::cout << date_str << std::endl;

	date = scale_indexer_weekly->GetIndexTime();
	date_str = TimeToString(date, "%Y-%m-%d");
	std::cout << date_str << std::endl;

}