#include <gtest/gtest.h>
#include "import.h"
#include "store.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class ImportTest : public testing::Test
{
public:
	KCStore store;
	Importer *importer;
	int count;

	virtual void SetUp()
	{
		importer = new Importer(&store);
		bool ret = store.Open(kDataPath + "TestData8.kch");
		std::cout << "ImportTest setup" << std::endl;
	}

	virtual void TearDown()
	{
		store.Close();
		std::cout << "ImportTest teardown" << std::endl;
	}
};

TEST_F(ImportTest, testVerifyNumber)
{
	count = importer->Import(kDataPath + "SH600623.csv");	
	HLOCList *hloc_list = new HLOCList();
	store.GetAll("SH600623", hloc_list);
	EXPECT_EQ(count, hloc_list->size());
}

TEST_F(ImportTest, testImport)
{
	count = importer->Import(kDataPath + "SH600633.csv");
	std::cout << count << std::endl;
	EXPECT_GT(count, 0);
}

TEST_F(ImportTest, testImportDir)
{
	count = importer->ImportDir(kDataPath + "raw\\");
	std::cout << count << " files imported."<< std::endl;
	EXPECT_GT(count, 0);
}

TEST_F(ImportTest, testImportBlock)
{
	BlockImporter importer(&store);
	count = importer.ImportDir(kDataPath + "blocks\\");
	std::cout << count << " files imported." << std::endl;
	EXPECT_GT(count, 0);

	std::string blocks;
	bool ret = store.GetBlockList(blocks);
	EXPECT_TRUE(ret);
	std::cout << "blocks: " << blocks << std::endl;
	std::cout << "blocks size: " << blocks.size() << std::endl;

	std::vector<std::string> elems;
	kyotocabinet::strsplit(blocks, '\n', &elems);
	std::string symbols;
	ret = store.GetBlock(elems[10], symbols);
	std::cout << "block name: " << elems[10] << std::endl;
	EXPECT_TRUE(ret);
	std::cout << "symbols: " << symbols << std::endl;

	ret = store.GetBlock("其它板块\\A股板块", symbols);
	std::cout << "block name: " << "其它板块\\A股板块" << std::endl;
	EXPECT_TRUE(ret);
	std::cout << "symbols: " << symbols << std::endl;

}

TEST_F(ImportTest, testGetAll)
{
	HLOCList *hloc_list = new HLOCList();
	bool ret = store.GetAll("SH600623", hloc_list);
	EXPECT_TRUE(ret);
}

TEST(StoreTest, testStoreOpenClose)
{
	KCStore store;
	Importer *importer = new Importer(&store);

	bool ret = store.Open(kDataPath + "TestData.kch");
	EXPECT_TRUE(ret);

	store.Close();

}

// try some parameterized testing...
class ImportFileListTest : public testing::TestWithParam<std::string>
{
public:
	KCStore store;
	Importer *importer;
	int count;

	virtual void SetUp()
	{
		importer = new Importer(&store);
		bool ret = store.Open(kDataPath + "TestData.kch");
		std::cout << "ImportFileListTest setup" << std::endl;
	}

	virtual void TearDown()
	{
		store.Close();
		std::cout << "ImportFileListTest teardown" << std::endl;
	}
};

INSTANTIATE_TEST_CASE_P(MyImportFileListTest,
	ImportFileListTest,
	::testing::Values("SH600623.csv", "SH600633.csv", "SH600643.csv", "SH600653.csv", "SH600663.csv"));

TEST_P(ImportFileListTest, importFileList)
{
	// Test loading a list of files...
	count = importer->Import(kDataPath + GetParam());
	std::cout << count << std::endl;
	EXPECT_GT(count, 0);
}

