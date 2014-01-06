// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// for strategy implementation.

#include "strategy.h"
#include "rule.h"
#include <ctime>
#include <assert.h>

namespace prism {
	
	Strategy::Strategy(const std::string& name): name_(name)
	{
		attach_rule_ = nullptr;
		detach_rule_ = nullptr;
	}

	Strategy::~Strategy()
	{
	}
	
	void Strategy::Serialize(JsonSerializer* serializer)
	{
		serializer->StartObject();
		serializer->String("description");
		serializer->String(description_.c_str());
		serializer->String("author");
		serializer->String(author_.c_str());
		serializer->String("version");
		serializer->String(version_.c_str());
		std::string begin = TimeToString(begin_time_, "%Y-%m-%d");
		serializer->String("begin_time");
		serializer->String(begin.c_str());
		std::string end = TimeToString(end_time_, "%Y-%m-%d");
		serializer->String("end_time");
		serializer->String(end.c_str());
		serializer->String("step");
		serializer->Int(step_);	
		serializer->String("init_cash");
		serializer->Double(init_cash_);
		serializer->String("num_portfolios");
		serializer->Int(num_portfolios_);
		serializer->String("stocks");
		serializer->String(stocks_.c_str());
		// the attach/detach rule
		serializer->String("attach_rule");
		serializer->StartObject();
		attach_rule_->Serialize(serializer);
		serializer->EndObject();
		serializer->String("detach_rule");
		serializer->StartObject();
		detach_rule_->Serialize(serializer);
		serializer->EndObject();

		serializer->EndObject();
	}

	bool Strategy::Parse(JsonDoc* json)
	{
		description_ = json->operator[]("description").GetString();
		version_ = json->operator[]("version").GetString();
		author_ = json->operator[]("author").GetString();
		init_cash_ = json->operator[]("init_cash").GetDouble();
		num_portfolios_ = json->operator[]("num_portfolios").GetInt();
		step_ = json->operator[]("step").GetInt();
		std::string begin = json->operator[]("begin_time").GetString();
		begin_time_ = StringToDate(begin, "%d-%d-%d");
		std::string end = json->operator[]("end_time").GetString();
		end_time_ = StringToDate(end, "%d-%d-%d");
		stocks_ = json->operator[]("stocks").GetString();

		JsonValue& jsonRule = json->operator[]("attach_rule");
		assert(jsonRule.IsObject());
		attach_rule_ = RuleFactory::CreateRule(&jsonRule);
		jsonRule = json->operator[]("detach_rule");
		assert(jsonRule.IsObject());
		detach_rule_ = RuleFactory::CreateRule(&jsonRule);
		return true;
	}


}