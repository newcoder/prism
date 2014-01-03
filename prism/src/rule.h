// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes rules for screening assets

#ifndef RULES_H
#define RULES_H

#include "common.h"
#include "util.h"
#include "time.h"
#include <string>
#include <vector>

namespace prism {

	class AssetIndexer;

	typedef enum
	{
		RULE_TYPE_NULL = 0,
		RULE_TYPE_AND,
		RULE_TYPE_OR,
		RULE_TYPE_TRUE,
		RULE_TYPE_FALSE,
		RULE_TYPE_INDICATOR_EMA,
		RULE_TYPE_INDICATOR_EMA_COMPARE,
		RULE_TYPE_INDICATOR_MACD,
		RULE_TYPE_INDICATOR_EMAARRAY,
		RULE_TYPE_INDICATOR_CR
	} RULE_TYPE;

	class IRule
	{
	public:
		virtual ~IRule() {}
		virtual void Serialize(JsonSerializer* serializer) = 0;
		virtual bool Parse(JsonValue* json) = 0;
		virtual bool Verify(AssetIndexer& asset_indexer) = 0;
		virtual RULE_TYPE type() = 0;
	};

	typedef std::vector<std::shared_ptr<IRule>> RuleList;

	class Rule: public IRule
	{
	public:
		Rule(RULE_TYPE type) : type_(type) {}
		virtual ~Rule() {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer) { return true; }
		// getter
		virtual RULE_TYPE type() { return type_; }
	public:
		static std::string TypeToStr(RULE_TYPE type);
		static RULE_TYPE StrToType(const std::string& type);
	protected:
		RULE_TYPE type_;
	};

	class TrueRule : public Rule
	{
	public:
		TrueRule();
		virtual bool Verify(AssetIndexer& asset_indexer) { return true; }
	};

	class FalseRule : public Rule
	{
	public:
		FalseRule();
		virtual bool Verify(AssetIndexer& asset_indexer) { return false; }
	};

	class RuleGroup : public Rule
	{
	public:
		RuleGroup(RULE_TYPE type);
		virtual ~RuleGroup();
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer) = 0;
		void Clear();
	protected:
		RuleList rules_;
	};

	class OrGroup : public RuleGroup
	{
	public:
		OrGroup();
		virtual bool Verify(AssetIndexer& asset_indexer);
	};

	class AndGroup : public RuleGroup
	{
	public:
		AndGroup();
		virtual bool Verify(AssetIndexer& asset_indexer);
	};

	class IndicatorRule : public Rule
	{
	public:
		IndicatorRule(RULE_TYPE type) : Rule(type), data_type_(DATA_TYPE_DAILY), data_num_(1) {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		// getter
		DATA_TYPE data_type() { return data_type_; }
		int data_num() { return data_num_; }
		void set_data_type(DATA_TYPE data_type) { data_type_ = data_type; }
		void set_data_num(int data_num) { data_num_ = data_num; }
	protected:
		DATA_TYPE data_type_;
		int data_num_;
	};

	class EMARule : public IndicatorRule
	{
	public:
		EMARule();
		EMARule(int period);
		virtual ~EMARule() {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer);
	public:
		int period() { return period_; }
		void set_period(int period) { period_ = period; }
	private:
		int period_;
	};

	class EMACompareRule : public IndicatorRule
	{
	public:
		EMACompareRule();
		EMACompareRule(int period_one, int period_two);
		virtual ~EMACompareRule() {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer);
	public:
		int period_one() { return period_one_; }
		int period_two() { return period_two_; }
		void set_period_one(int period_one) { period_one_ = period_one; }
		void set_period_two(int period_two) { period_two_ = period_two; }
	private:
		int period_one_;
		int period_two_;
	};

	class MACDRule : public IndicatorRule
	{
	public:
		MACDRule();
		MACDRule(int short_period, int long_period, int signal_period);
		virtual ~MACDRule() {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer);
	public:
		int short_period() { return short_period_; }
		int long_period() { return long_period_; }
		int signal_period() { return signal_period_; }
		bool linear_predict() { return linear_predict_; }
		int look_back() { return look_back_; }
		double threshold() { return threshold_; }
		void set_short_period(int short_period) { short_period_ = short_period; }
		void set_long_period(int long_period) { long_period_ = long_period; }
		void set_signal_period(int signal_period) { signal_period_ = signal_period; }
		void set_linear_predict(bool linear_predict) { linear_predict_ = linear_predict; }
		void set_look_back(int look_back) { look_back_ = look_back; }
		void set_threshold(double threshold) { threshold_ = threshold; }
	private:
		int short_period_;
		int long_period_;
		int signal_period_;
		bool linear_predict_;
		int look_back_;
		double threshold_;
	};

	class EMAArrayRule : public IndicatorRule
	{
	public:
		EMAArrayRule();
		EMAArrayRule(int first_period, int second_period, int third_period, int fourth_period);
		virtual ~EMAArrayRule() {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer);
	private:
		int first_period_;
		int second_period_;
		int third_period_;
		int fourth_period_;
	};

	class CRRule : public IndicatorRule
	{
	public:
		CRRule();
		CRRule(int period);
		virtual ~CRRule() {}
		virtual void Serialize(JsonSerializer* serializer);
		virtual bool Parse(JsonValue* json);
		virtual bool Verify(AssetIndexer& asset_indexer);
	public:
		int period() const { return period_; }
		void set_period(int period) { period_ = period; }
	private:
		int period_;
	};

	// Rule factory
	class RuleFactory
	{
	public:
		RuleFactory();
		~RuleFactory();
		static std::shared_ptr<IRule> CreateRuleType(RULE_TYPE type);
		static std::shared_ptr<IRule> CreateRule(JsonValue* json);
	};

}

#endif