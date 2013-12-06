// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// utility functions

#ifndef UTIL_H
#define UTIL_H

#include <time.h>
#include <vector>
#include <string>

// rapidjson
#include "rapidjson/document.h"		// rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"	// for stringify JSON
#include "rapidjson/filestream.h"	// wrapper of C stream for prettywriter as output
#include <cstdio>

namespace prism {

	// date time ..
	std::string TimeToString(time_t time_stamp, const char* format);
	time_t StringToTime(const std::string& time_str, const char* format);
	time_t StringToDate(const std::string& time_str, const char* format);
	bool InSameWeek(time_t d1, time_t d2);
	bool InSameMonth(time_t d1, time_t d2);
	size_t GetYear(time_t time_stamp);

	// json parser&serializer, based on rapidjson
	typedef rapidjson::Value JsonValue;
	typedef rapidjson::Document JsonDoc;
	typedef rapidjson::PrettyWriter<rapidjson::FileStream> JsonSerializer;

	bool ParseJson(const std::string& path, JsonDoc& doc);	
	
	class JsonArchive
	{
	public:
		JsonArchive(const std::string& path);
		~JsonArchive();
		bool Open();
		void Close();
		JsonSerializer* GetSerializer();
	private:
		std::string path_;
		FILE *file_;
		rapidjson::FileStream* file_stream_;
		JsonSerializer* writer_;
	};

}

#endif
