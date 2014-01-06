// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for strategy
// strategy is about execute actions based on a rule...

#ifndef STRATEGY_H
#define STRATEGY_H

#include "common.h"
#include "util.h"
#include "time.h"
#include <string>

namespace prism {
	
	class IRule;

	/*
	"description": "sample for strategy",
	"author": "rayni",
	"version": "0.1",
	"begin_time": "1992-01-01",
	"end_time": "2012-12-31",
	"step": 1,
	"init_cash": 1000000,
	"stocks": "patterns:SH600\\d{3};blocks=XXXXX;",
	"attach_rule": {},
	"detach_rule": {}
	**/

	class Strategy
	{
	public:
		Strategy(const std::string& name);
		~Strategy();
		void Serialize(JsonSerializer* serializer);
		bool Parse(JsonDoc* json);
	public:
		// getter
		std::string name() const { return name_; }
		std::string description() const { return description_; }
		std::string version() const { return version_; }
		std::string author() const { return author_; }
		double init_cash() const { return init_cash_; }
		int num_portfolios() const { return num_portfolios_; }
		int step() const { return step_; }
		time_t begin_time() const { return begin_time_; }
		time_t end_time() const { return end_time_; }
		std::string stocks() const { return stocks_; }
		IRule* attach_rule() const { return attach_rule_.get(); }
		IRule* detach_rule() const { return detach_rule_.get(); }
		// setter
		void set_name(const std::string& name) { name_ = name; }
		void set_description(const std::string& description) { description_ = description; }
		void set_version(const std::string& version) { version_ = version; }
		void set_author(const std::string& author) { author_ = author; }
		void set_init_cash(double init_cash) { init_cash_ = init_cash; }
		void set_num_portfolios(int num_portfolios) { num_portfolios_ = num_portfolios; }
		void set_step(int step) { step_ = step; }
		void set_begin_time(time_t begin_time) { begin_time_ = begin_time; }
		void set_end_time(time_t end_time) { end_time_ = end_time; }
		void set_stocks(const std::string& stocks) { stocks_ = stocks; }
		void set_attach_rule(const std::shared_ptr<IRule>& attach_rule) { attach_rule_ = attach_rule; }
		void set_detach_rule(const std::shared_ptr<IRule>& detach_rule) { detach_rule_ = detach_rule; }
	private:
		std::string name_;
		std::string description_;
		std::string version_;
		std::string author_;
		double init_cash_;
		int num_portfolios_;
		int step_;
		time_t begin_time_;
		time_t end_time_;
		std::string stocks_;
		std::shared_ptr<IRule> attach_rule_;
		std::shared_ptr<IRule> detach_rule_;
	};


}

#endif