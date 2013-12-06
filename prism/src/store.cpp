// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// centric data storage.

#include "store.h"
#include "util.h"
#include <ctime>

namespace prism {

	KCStore::KCStore()
	{

	}

	KCStore::~KCStore()
	{
	}

	bool KCStore::Open(const std::string& file)
	{
		// open the database
		if (!db_.open(file, kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE))
			return false;
		return true;
	}

	void KCStore::Close()
	{
		db_.close();
	}

	std::string KCStore::GenerateKey(const std::string& symbol, const time_t time)
	{
		return symbol + TimeToString(time, "_%Y");
	}
	
	std::string KCStore::GenerateLastKey(const std::string& symbol)
	{
		return symbol + "_LAST_POINT";
	}

	std::string KCStore::GenerateFirstKey(const std::string& symbol)
	{
		return symbol + "_FIRST_POINT";
	}

	bool KCStore::GetLast(const std::string& symbol, HLOC* point)
	{
		std::string key = GenerateLastKey(symbol);
		return -1 != db_.get(key.c_str(), key.size(), (char *)point, sizeof(HLOC));
	}
	
	bool KCStore::GetFirst(const std::string& symbol, HLOC* point)
	{
		std::string key = GenerateFirstKey(symbol);
		return -1 != db_.get(key.c_str(), key.size(), (char *)point, sizeof(HLOC));
	}

	bool KCStore::UpdateLast(const std::string& symbol, HLOC* point)
	{
		std::string key = GenerateLastKey(symbol);
		return db_.set(key.c_str(), key.size(), (const char*)point, sizeof(HLOC));
	}

	bool KCStore::UpdateFirst(const std::string& symbol, HLOC* point)
	{
		std::string key = GenerateFirstKey(symbol);
		return db_.set(key.c_str(), key.size(), (const char*)point, sizeof(HLOC));
	}

	bool KCStore::Append(const std::string& symbol, HLOCList* points)
	{
		std::string key = GenerateKey(symbol, points->at(0).time);
		return db_.append(key.c_str(), key.size(), (const char*)&points->at(0), sizeof(HLOC)*points->size());
	}

	bool KCStore::Put(const std::string& symbol, HLOCList *points)
	{
		HLOC last;
		if (points->empty()) return true;
		// check first point
		std::string key = GenerateFirstKey(symbol);
		if (db_.check(key) == -1)
		{
			// first time
			return Append(symbol, points) &&
				UpdateFirst(symbol, &points->at(0)) &&
				UpdateLast(symbol, &points->at(points->size() - 1));
		}

		if (GetLast(symbol, &last)) {
			if (last.time > points->at(0).time)
			{
				return false;
			}
			else
			{
				return Append(symbol, points) &&
					UpdateLast(symbol, &points->at(points->size() - 1));
			}
		}

		return false;
	}

	bool KCStore::Get(const std::string& symbol, size_t begin_year, size_t end_year, HLOCList* result)
	{	
		int32_t vsize;
		HLOC vbuf[300];
		result->clear();
		for (size_t i = begin_year; i <= end_year; i++)
		{
			std::string key = symbol + "_" + std::to_string(static_cast<long long>(i));
			// if there is no data for this year, continue...
			if (db_.check(key) == -1)
				continue;
			vsize = db_.get(key.c_str(), key.size(), (char*)vbuf, 300*sizeof(HLOC));
			if (vsize == -1)
			{
				// failure
				return false;
			}
			assert(vsize % sizeof(HLOC) == 0);
			if (vsize > 0)
				result->insert(result->end(), &vbuf[0], &vbuf[vsize / sizeof(HLOC)]);
		}
		return true;
	}

	bool KCStore::GetAll(const std::string& symbol, HLOCList* result)
	{
		HLOC first, last;
		if (GetFirst(symbol, &first) && GetLast(symbol, &last))
		{
			std::tm timeInfo;
			localtime_s(&timeInfo, &(first.time));
			size_t yearBegin = timeInfo.tm_year + 1900;
			localtime_s(&timeInfo, &(last.time));
			size_t yearEnd = timeInfo.tm_year + 1900;
			return Get(symbol, yearBegin, yearEnd, result);
		}
		return false;
	}

	bool KCStore::GetBlockList(std::string& block_list)
	{
		std::string key = "ALL_BLOCKS";
		char vbuf[102400];
		memset(vbuf, 0, 102400);
		int32_t vsize = db_.get(key.c_str(), key.size(), (char*)vbuf, sizeof(char)*102400);
		if (vsize == -1)
			return false;
		block_list = std::string(vbuf);
		return true;
	}

	bool KCStore::PutBlock(const std::string& block_name, const std::string& symbols)
	{
		if (db_.check(block_name.c_str()) == -1)
		{
			// not exist, append the block name to ALL_BLOCKS
			std::string key = "ALL_BLOCKS";
			std::string value = block_name + "\n";
			db_.append(key.c_str(), key.size(), (const char*)value.c_str(), value.size());
		}
		return db_.set(block_name.c_str(), block_name.size(), (const char*)symbols.c_str(), symbols.size());

	}

	bool KCStore::GetBlock(const std::string& block_name, std::string& symbols)
	{
		char vbuf[102400];
		memset(vbuf, 0, 102400);
		int32_t vsize = db_.get(block_name.c_str(), block_name.size(), (char*)vbuf, sizeof(char)* 102400);
		if (vsize == -1)
			return false;
		symbols = std::string(vbuf);
		return true;
	}

}