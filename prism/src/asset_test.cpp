#include <gtest/gtest.h>
#include "asset.h"
#include "store.h"
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
	
	AssetsLoader loader((IStore*)&store);
	int count = loader.LoadAssets(elems, 2011, 2013);
	std::cout << "symbols loaded: " << count << std::endl;
}

