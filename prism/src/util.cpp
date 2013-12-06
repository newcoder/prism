#include "util.h"
#include <ctime>
#include <algorithm>
#include <fstream>

namespace prism
{
	std::string TimeToString(time_t time_stamp, const char* format)
	{
		std::tm timeinfo;
		char buffer[80];

		localtime_s(&timeinfo, &time_stamp);
		std::strftime(buffer, 80, format, &timeinfo);
		return std::string(buffer);
	}

	time_t StringToTime(const std::string& time_str, const char* format)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		int yyyy, mon, dd, hh, mm, ss;

		sscanf_s(time_str.c_str(), format, &yyyy, &mon, &dd, &hh, &mm, &ss);
		tm.tm_year = yyyy - 1900;
		tm.tm_mon = mon - 1;
		tm.tm_mday = dd;
		tm.tm_hour = hh;
		tm.tm_min = mm;
		tm.tm_sec = ss;
		return mktime(&tm);
	}
	
	time_t StringToDate(const std::string& time_str, const char* format)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		int yyyy, mon, dd;

		sscanf_s(time_str.c_str(), format, &yyyy, &mon, &dd);
		tm.tm_year = yyyy - 1900;
		tm.tm_mon = mon - 1;
		tm.tm_mday = dd;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
		return mktime(&tm);
	}

	bool InSameWeek(time_t d1, time_t d2)
	{
		std::tm tm1, tm2;
		if (std::abs(d1 - d2) > 604800) // 7*24*3600 seconds
			return false;	
		localtime_s(&tm1, &d1);
		localtime_s(&tm2, &d2);
		return tm1.tm_wday >= tm2.tm_wday? d1 >= d2 : d1 < d2;
	}

	bool InSameMonth(time_t d1, time_t d2)
	{
		std::tm tm1, tm2;
		localtime_s(&tm1, &d1);
		localtime_s(&tm2, &d2);
		return tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon;
	}

	size_t GetYear(time_t time_stamp)
	{
		std::tm timeinfo;
		localtime_s(&timeinfo, &time_stamp);
		return timeinfo.tm_year + 1900;
	}

	bool ParseJson(const std::string& path, JsonDoc& doc)
	{
		std::ifstream ifs(path);
		std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		if (doc.Parse<0>(json.c_str()).HasParseError())
			return false;
		assert(doc.IsObject());
		return true;
	}

	JsonArchive::JsonArchive(const std::string& path) : path_(path) 
	{
	}
	
	JsonArchive::~JsonArchive() 
	{
	}
	
	bool JsonArchive::Open()
	{
		fopen_s(&file_, path_.c_str(), "w");
		if (file_ == NULL)
			return false;
		file_stream_ = new rapidjson::FileStream(file_);
		writer_ = new JsonSerializer(*file_stream_);
		return true;
	}

	void JsonArchive::Close() 
	{
		delete file_stream_;
		delete writer_;
		fclose(file_);
	}

	JsonSerializer* JsonArchive::GetSerializer()
	{
		assert(writer_ != NULL);
		return writer_;
	}

}