// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// Import data from various sources to the centric data storage.

#ifndef IMPORT_H
#define IMPORT_H

#include "common.h"
#include <string>

namespace prism {

	class HLOC;
	class IStore;

	// getting a HLOC point from one line of CSV file
	// for WeiSheng csv file format
	class WSCSVConverter {
	public:
		bool Convert(const std::string& line, HLOC* point, int* year);
	};

	// filter the files
	class AShareFilter {
	public:
		bool Filter(const std::string& file);
	};

	// import data from csv file to data store 
	// Converter: convert csv line to HLOC record
	// Filter:	  filter csv files in the directory
	template <typename C, typename F>
	class CSVImporter {
	public:
		CSVImporter(std::shared_ptr<IStore> store);
		~CSVImporter();
	public:
		// import the csv file to data store
		// return count of lines on success
		// return -1 if open file failed
		// return -2 if format invalid
		// return -3 if access data store failed
		int Import(const std::string& csvfile);
		// import all csv files in the folder
		// return the number of imported files
		int ImportDir(const std::string& root);
	private:
		std::string GetSymbol(const std::string& file_name);
	private:
		std::shared_ptr<IStore> store_;
		C converter_;
		F filter_;
	};

	typedef CSVImporter<WSCSVConverter, AShareFilter> Importer;

	// TODO: import the blocks data to data store, °å¿éÐÅÏ¢
	class BlockImporter
	{
	public:
		BlockImporter(std::shared_ptr<IStore> store);
		~BlockImporter();
	public:
		// import all blocks data in the folder
		// return the number of imported blocks
		int ImportDir(const std::string& root);
	private:
		std::shared_ptr<IStore> store_;
	};
}

#endif
