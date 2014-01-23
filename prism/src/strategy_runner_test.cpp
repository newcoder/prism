#include <gtest/gtest.h>
#include "strategy_runner.h"
#include "store.h"
#include "time_series.h"
#include <iostream>

using namespace prism;

const std::string kDataPath = "D:\\project\\prism\\data\\";

class StrategyRunnerTest : public testing::Test
{
public:
	std::shared_ptr<KCStore> store_;
	std::shared_ptr<Strategy> strategy_;
	std::shared_ptr<AssetsProvider> assets_provider_;
	std::shared_ptr<PortfolioManager> portfolio_manager_;
	std::shared_ptr<TransactionManager> transaction_manager_;
	std::shared_ptr<RunnerObserver> runner_observer_;
	std::shared_ptr<StrategyRunner> runner_;

	virtual void SetUp()
	{
		store_ = std::make_shared<KCStore>();
		bool ret = store_->Open(kDataPath + "TestData8.kch");
		EXPECT_TRUE(ret);

		strategy_ = std::make_shared<Strategy>("test strategy");
		assets_provider_ = std::make_shared<AssetsProvider>(store_);
		portfolio_manager_ = std::make_shared<PortfolioManager>();
		transaction_manager_ = std::make_shared<TransactionManager>();
		runner_observer_ = std::make_shared<RunnerObserver>();

		runner_ = std::make_shared<StrategyRunner>(strategy_, 
			assets_provider_, 
			portfolio_manager_, 
			transaction_manager_, 
			runner_observer_);
	}

	virtual void TearDown()
	{
		store_->Close();
	}
};

TEST_F(StrategyRunnerTest, testSimple)
{
	prism::JsonDoc doc;
	bool ret = prism::ParseJson(kDataPath + "strategy_simple.txt", doc);
	EXPECT_TRUE(ret);
	ret = strategy_->Parse(&doc);
	EXPECT_TRUE(ret);

	std::string symbol = "SH600196";
	std::vector<std::string> elems;
	elems.push_back(symbol);
	int count = assets_provider_->LoadAssets(elems, 2011, 2013);
	assert(count == 1);

	runner_->Init();
	runner_->Run();
	runner_->Clear();
}