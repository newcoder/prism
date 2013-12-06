// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// classes to manage the centric storage.

#ifndef STORE_H
#define STORE_H

#include <string>
#include <time.h>
#include "common.h"

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4351 )
#pragma warning( disable : 4996 )
#include <kcpolydb.h>
#pragma warning( pop ) 


namespace prism {


	// interface for generic HLOC data store
	class IStore {
	public:
		virtual ~IStore(){};
		virtual bool Put(const std::string& symbol, HLOCList* points) = 0;
		virtual bool Get(const std::string& symbol, size_t begin_year, size_t end_year, HLOCList* result) = 0;
		virtual bool GetAll(const std::string& symbol, HLOCList* result) = 0;
		virtual bool GetLast(const std::string& symbol, HLOC* point) = 0;
		virtual bool GetFirst(const std::string& symbol, HLOC* point) = 0;
		virtual bool GetBlockList(std::string& block_list) = 0;
		virtual bool PutBlock(const std::string& block_name, const std::string& symbols) = 0;
		virtual bool GetBlock(const std::string& block_name, std::string& symbols) = 0;
	};

	// Kyoto Cabinet data store
	// the structure, for each year, there is a key/pair to store the points in that year.
	//		key: SYMBOL_2013
	//		value: All HLOC points in 2013
	// for each symbol, there is a key/pair to store the first point and a key/pair to store the last point
	//		key: SYMBOL_FIRST_POINT, SYMBOL_LAST_POINT
	//		value: the first / last HLOC point
	// 
	// the block data
	//		key: blockname, if the name is multi-level, separated by '\', e.g. ÐÐÒµ\½ðÈÚ
	//		value: all symbols in the block splitted by '\n', e.g. SH600012\nSH600434
	// there is a key/pair stores all block names
	//      key: ALL_BLOCKS
	//      value: all block names, separated by \n
	class KCStore: public IStore
	{
	public:
		KCStore();
		virtual ~KCStore();
	public:
		bool Open(const std::string& file);
		void Close();

		//overrides
		// put points to data store:
		// 1. the points should be in same year.
		// 2. the point's time should be greater than the LAST POINT 
		bool Put(const std::string& symbol, HLOCList* points);
		bool Get(const std::string& symbol, size_t begin_year, size_t end_year, HLOCList* result);
		bool GetAll(const std::string& symbol, HLOCList* result);
		bool GetLast(const std::string& symbol, HLOC* point);
		bool GetFirst(const std::string& symbol, HLOC* point);
		// blocks
		bool GetBlockList(std::string& block_list);
		bool PutBlock(const std::string& block_name, const std::string& symbols);
		bool GetBlock(const std::string& block_name, std::string& symbols);
	private:		
		std::string GenerateKey(const std::string& symbol, const time_t time);
		std::string GenerateLastKey(const std::string& symbol);
		std::string GenerateFirstKey(const std::string& symbol);
		bool UpdateLast(const std::string& symbol, HLOC* point);
		bool UpdateFirst(const std::string& symbol, HLOC* point);
		bool Append(const std::string& symbol, HLOCList* points);
	private:
		kyotocabinet::PolyDB db_;
	};

}

#endif