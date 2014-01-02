#include <gtest/gtest.h>
#include "import.h"
#include "store.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class ImportTest : public testing::Test
{
public:
	//std::shared_ptr<int> test(std::make_shared<int>);
	std::shared_ptr<KCStore> store_;
	std::shared_ptr<Importer> importer_;
	int count;

	virtual void SetUp()
	{
		store_ = std::make_shared<KCStore>();
		importer_ = std::make_shared<Importer>(store_);
		bool ret = store_->Open(kDataPath + "TestData8.kch");
		std::cout << "ImportTest setup" << std::endl;
	}

	virtual void TearDown()
	{
		store_->Close();
		std::cout << "ImportTest teardown" << std::endl;
	}
};

TEST_F(ImportTest, testVerifyNumber)
{
	count = importer_->Import(kDataPath + "SH600623.csv");	
	auto hloc_list = std::make_shared<HLOCList>();
	store_->GetAll("SH600623", hloc_list);
	EXPECT_EQ(count, hloc_list->size());
}

TEST_F(ImportTest, testImport)
{
	count = importer_->Import(kDataPath + "SH600633.csv");
	std::cout << count << std::endl;
	EXPECT_GT(count, 0);
}

TEST_F(ImportTest, testImportDir)
{
	count = importer_->ImportDir(kDataPath + "raw\\");
	std::cout << count << " files imported."<< std::endl;
	EXPECT_GT(count, 0);
}

TEST_F(ImportTest, testImportBlock)
{
	BlockImporter importer(store_);
	count = importer.ImportDir(kDataPath + "blocks\\");
	std::cout << count << " files imported." << std::endl;
	EXPECT_GT(count, 0);

	std::string blocks;
	bool ret = store_->GetBlockList(blocks);
	EXPECT_TRUE(ret);
	std::cout << "blocks: " << blocks << std::endl;
	std::cout << "blocks size: " << blocks.size() << std::endl;

	std::vector<std::string> elems;
	kyotocabinet::strsplit(blocks, '\n', &elems);
	std::string symbols;
	ret = store_->GetBlock(elems[10], symbols);
	std::cout << "block name: " << elems[10] << std::endl;
	EXPECT_TRUE(ret);
	std::cout << "symbols: " << symbols << std::endl;

	ret = store_->GetBlock("其它板块\\A股板块", symbols);
	std::cout << "block name: " << "其它板块\\A股板块" << std::endl;
	EXPECT_TRUE(ret);
	std::cout << "symbols: " << symbols << std::endl;

}

TEST_F(ImportTest, testGetAll)
{
	auto hloc_list = std::make_shared<HLOCList>();
	bool ret = store_->GetAll("SH600623", hloc_list);
	EXPECT_TRUE(ret);
}

TEST(StoreTest, testStoreOpenClose)
{
	std::shared_ptr<KCStore> store = std::make_shared<KCStore>();
	std::unique_ptr<Importer> importer = std::make_unique<Importer>(store);

	bool ret = store->Open(kDataPath + "TestData.kch");
	EXPECT_TRUE(ret);

	store->Close();

}

// try some parameterized testing...
class ImportFileListTest : public testing::TestWithParam<std::string>
{
public:
	std::shared_ptr<KCStore> store_;
	std::shared_ptr<Importer> importer_;
	int count;

	virtual void SetUp()
	{
		store_ = std::make_shared<KCStore>();
		importer_ = std::make_shared<Importer>(store_);
		bool ret = store_->Open(kDataPath + "TestData.kch");
		std::cout << "ImportFileListTest setup" << std::endl;
	}

	virtual void TearDown()
	{
		store_->Close();
		std::cout << "ImportFileListTest teardown" << std::endl;
	}
};

INSTANTIATE_TEST_CASE_P(MyImportFileListTest,
	ImportFileListTest,
	::testing::Values("SH600623.csv", "SH600633.csv", "SH600643.csv", "SH600653.csv", "SH600663.csv"));

TEST_P(ImportFileListTest, importFileList)
{
	// Test loading a list of files...
	count = importer_->Import(kDataPath + GetParam());
	std::cout << count << std::endl;
	EXPECT_GT(count, 0);
}

