#include <gtest/gtest.h>
#include "asset.h"
#include "store.h"
#include "util.h"
#include "trade.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class TradeTest : public testing::Test
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


TEST_F(TradeTest, testTransactions)
{
	AssetsProvider loader((IStore*)&store);

	std::vector<std::string> elems{ "SH600198", "SZ002015" };
	int count = loader.LoadAssets(elems, 2011, 2013);

	Asset* asset = loader.Get("SH600198");
	AssetIndexer *asset_indexer = new AssetIndexer(asset);

	TransactionManager trans_manager;
	Transaction trans;
	trans.asset_indexer_ = asset_indexer;
	trans.commission_ = 0.001;
	trans.price_ = 12.01;
	trans.shares_ = 10000;
	trans.time_ = -1;
	trans.type_ = TRANSACTION_TYPE_BUY;

	int num_trans = 123;
	for (int i = 0; i < 123; i++)
		trans_manager.Add(trans);
	EXPECT_EQ(TransactionManager::id, num_trans);

	asset = loader.Get("SZ002015");
	asset_indexer = new AssetIndexer(asset);
	trans.asset_indexer_ = asset_indexer;
	for (int i = 0; i < 152; i++)
		trans_manager.Add(trans);

	TransactionList trans_list;
	trans_manager.GetTransactions("SH600198", &trans_list);
	EXPECT_EQ(trans_list.size(), num_trans);

}
TEST_F(TradeTest, testPortfolios)
{
	AssetsProvider loader((IStore*)&store);
	std::string symbol1 = "SH600198";
	std::string symbol2 = "SZ002015";

	std::vector<std::string> elems{symbol1, symbol2 };
	int count = loader.LoadAssets(elems, 2011, 2013);

	Asset* asset1 = loader.Get(symbol1);
	AssetIndexer *asset_indexer1 = new AssetIndexer(asset1);

	Asset* asset2 = loader.Get(symbol2);
	AssetIndexer *asset_indexer2 = new AssetIndexer(asset2);

	PortfolioManager portfolio_manager;
	portfolio_manager.Buy(asset_indexer1, 14000);
	portfolio_manager.Buy(asset_indexer2, 21000);
	portfolio_manager.Buy(asset_indexer1, 16000);
	portfolio_manager.Buy(asset_indexer2, 25000);
	portfolio_manager.Sell(asset_indexer1, 10000);

	Portfolio* item = portfolio_manager.Get(symbol1);
	EXPECT_DOUBLE_EQ(item->amount(), 20000);

	time_t day = StringToDate("2012-12-21", "%d-%d-%d");
	double value = 0.0f;
	bool ret = portfolio_manager.GetValue(value, day);
	std::cout << "value: " << value << std::endl;
	double value1;
	ret = portfolio_manager.GetValue(value1);
	std::cout << "value: " << value1 << std::endl;
	EXPECT_DOUBLE_EQ(value, value1);

	asset_indexer1->ToEnd();
	asset_indexer2->ToEnd();
	ret = portfolio_manager.GetValue(value);
	std::cout << "value: " << value << std::endl;

}

