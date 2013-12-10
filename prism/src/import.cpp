// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// Import data from various sources to the centric data storage.

#include "import.h"
#include "common.h"
#include "store.h"
#include "util.h"
#include <time.h>
#include <fstream>
#include <iostream>
#include <kcutil.h>
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"
using namespace boost::filesystem;

namespace prism {

	//微盛投资，csv格式
	//Date,Open,Low,High,Close,Volume,Amount
	//1993 - 03 - 04 00:00 : 00, 4.203, 4.203, 6.122, 4.784, 9224, 18098000.00
	bool WSCSVConverter::Convert(const std::string& line, HLOC* point, int* year)
	{
		std::vector<std::string> elems;
		size_t count = kyotocabinet::strsplit(line, ',', &elems);
		if (count != 7)
		{
			std::cout << line << std::endl;
			return false;
		}
		// timestamp
		point->time = StringToTime(elems[0], "%d-%d-%d %d:%d:%d");
		point->open = atof(elems[1].c_str());
		point->low = atof(elems[2].c_str());
		point->high = atof(elems[3].c_str());
		point->close = atof(elems[4].c_str());
		point->volume = atoi(elems[5].c_str());
		point->amount = atof(elems[6].c_str());

		kyotocabinet::strsplit(line, '-', &elems);
		*year = atoi(elems[0].c_str());

		return true;
	}

	bool AShareFilter::Filter(const std::string& file)
	{
		boost::regex pattern("^S[HZ]\\d{5}3\\.csv");
		return regex_match(file, pattern);
	}

	template <typename C, typename F>
	CSVImporter<C, F>::CSVImporter(IStore* store) : store_(store)
	{
	}

	template <typename C, typename F>
	CSVImporter<C, F>::~CSVImporter()
	{
	}

	template <typename C, typename F>
	std::string CSVImporter<C, F>::GetSymbol(const std::string& file_name)
	{
		std::vector<std::string> elems;
		kyotocabinet::strsplit(file_name, '\\', &elems);
		std::string file = elems[elems.size() - 1];
		kyotocabinet::strsplit(file, '.', &elems);
		return elems[0];
	}

	template <typename C, typename F>
	int CSVImporter<C, F>::Import(const std::string& csvfile)
	{
		std::ifstream infile;
		infile.open(csvfile);
		if (!infile)
			return -1;
			
		std::string symbol = GetSymbol(csvfile);
		std::string line;
		char buf[100];
		HLOCList group;
		int previous;
		int count = 0;
		// skip first line, the header
		infile.getline(buf, 100);
		while (!infile.eof())
		{
			infile.getline(buf,100);
			line = std::string(buf);
			if (line.empty())
				continue;

			HLOC point;
			int year;
			if (!converter_.Convert(line, &point, &year))
				return -2;

			if (group.empty())
			{
				group.push_back(point);
				previous = year;
			}
			else if (year == previous)
			{
				group.push_back(point);
			} 
			else 
			{
				if (!store_->Put(symbol, &group))
					return -3;
				group.clear();
				group.push_back(point);
				previous = year;
			}
			count++;
		}

		if (!store_->Put(symbol, &group))
			return -3;

		infile.close();
		return count;
	}

	template <typename C, typename F>
	int CSVImporter<C, F>::ImportDir(const std::string& root)
	{
		path current_dir(root); //
		int count = 0;
		for (recursive_directory_iterator iter(current_dir), end;
			iter != end;
			++iter)
		{
			std::string name = iter->path().leaf().string();
			if (filter_.Filter(name))
			{
				int ret = Import(iter->path().string());
				if (ret < 0)
				{
					// import failed.
					std::cerr << "Import Failed, error code: " << ret << ", file:" << iter->path() << "\n";
					return count;
				}
				std::cout << count << "： " << iter->path() << "\t\tdone!\n";
				count++;
			}
		}
		return count;
	}

	template class CSVImporter<WSCSVConverter, AShareFilter>;

	BlockImporter::BlockImporter(IStore* store) : store_(store)
	{
	}
	
	BlockImporter::~BlockImporter()
	{
	}

	int BlockImporter::ImportDir(const std::string& root)
	{
		path current_dir(root); //
		int count = 0;
		for (recursive_directory_iterator iter(current_dir), end;
			iter != end;
			++iter)
		{
			if (!is_regular_file(iter->path()))
				continue;

			// read the symbols
			std::ifstream ifs(iter->path().string());
			std::string symbols((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

			// extract the block name from the full path			
			std::string full = iter->path().string();
			// get rid of the root
			std::string block_name = full.substr(root.size());
			// get rid of the extension
			size_t pos = block_name.find_last_of('.');
			block_name = block_name.substr(0, pos);

			// save to data store
			if (!store_->PutBlock(block_name, symbols))
			{
				return -3;
			}
			std::cout << count << "： " << iter->path() << "\t\tdone!\n";
			count++;
		}
		return count;
	}

}
