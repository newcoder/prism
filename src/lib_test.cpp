#include <gtest/gtest.h>
#include <iostream>

#include "FFT.h"

// eigen
#define EIGEN2_SUPPORT
#include <Eigen/Core>
#include <Eigen/LeastSquares>
#include <unsupported/Eigen/FFT>

// boost 
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"

// rapidjson
#include "rapidjson/document.h"		// rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"	// for stringify JSON
#include "rapidjson/filestream.h"	// wrapper of C stream for prettywriter as output
#include <cstdio>

#include "util.h"

using namespace rapidjson;

TEST(LibTest, testFFT)
{
	typedef std::complex<double> cx;
	cx a[] = { cx(0,0), cx(1,0), cx(3,0), cx(4,0),
		cx(4, 0), cx(3, 0), cx(1,0), cx(0,0) };
	cx b[8];
	prism::FFT(a, b, 3);
	for (int i=0; i<8; ++i)
		std::cout << b[i] << "\n";

}

TEST(LibTest, testEigenDynamicSize)
{
	for (int size=1; size<=4; ++size)
	{
		Eigen::MatrixXi m(size,size+1); // a (size)x(size+1)-matrix of int's
		for (int j=0; j<m.cols(); ++j) // loop over columns
			for (int i=0; i<m.rows(); ++i) // loop over rows
				m(i,j) = i+j*m.rows(); // to access matrix coefficients,
		// use operator()(int,int)
		std::cout << m << "\n\n";
	}
	Eigen::VectorXf v(4); // a vector of 4 float's
	// to access vector coefficients, use either operator () or operator []
	v[0] = 1; v[1] = 2; v(2) = 3; v(3) = 4;
	std::cout << "\nv:\n" << v << std::endl;
}

TEST(LibTest, testEigenLinearRegression)
{
	Eigen::Vector3d points[5];
	points[0] = Eigen::Vector3d( 3.02, 6.89, -4.32 );
	points[1] = Eigen::Vector3d( 2.01, 5.39, -3.79 );
	points[2] = Eigen::Vector3d( 2.41, 6.01, -4.01 );
	points[3] = Eigen::Vector3d( 2.09, 5.55, -3.86 );
	points[4] = Eigen::Vector3d( 2.58, 6.32, -4.10 );


	std::vector<Eigen::Vector3d*> points_ptrs(5);
	for(int k=0; k<5; ++k) points_ptrs[k] = &points[k];
	Eigen::Vector3d coeffs; // will store the coefficients a, b, c
	Eigen::linearRegression(
		5,
		&(points_ptrs[0]),
		&coeffs,
		1 // the coord to express as a function of
		// the other ones. 0 means x, 1 means y, 2 means z.
		);
	std::cout << coeffs << std::endl;                            
}

//TEST(LibTest, testEigenFFT)
//{
//	Eigen::FFT<float> fft;
//
//	std::vector<float> timevec(8);
//	timevec[0] = 0;
//	timevec[1] = 1;
//	timevec[2] = 3;
//	timevec[3] = 4;
//	timevec[4] = 4;
//	timevec[5] = 3;
//	timevec[6] = 1;
//	timevec[7] = 0;
//
//	std::vector<std::complex<float> > freqvec;
//
//	fft.fwd( freqvec,timevec);
//	for (size_t i=0; i<freqvec.size(); ++i)
//		std::cout << freqvec[i] << "\n";	
//	
//	// manipulate freqvec
//	fft.inv( timevec,freqvec);
//	for (size_t i=0; i<timevec.size(); ++i)
//		std::cout << timevec[i] << "\n";
//}


using namespace boost::filesystem;
const std::string kDataPath = "D:\\project\\prism\\data\\";

TEST(LibTest, testBoostFile)
{
	path current_dir(kDataPath + "blocks\\"); //
	int count = 0;
	for (recursive_directory_iterator iter(current_dir), end;
		iter != end;
		++iter)
	{		
		if (!is_regular_file(iter->path()))
			continue;
		std::cout << ++count << ": " << iter->path() << "\n";
		std::cout << ++count << ": " << iter->path().relative_path() << "\n";
		std::cout << ++count << ": " << iter->path().filename() << "\n";
		std::cout << ++count << ": " << iter->path().root_path() << "\n";
		std::cout << ++count << ": " << iter->path().root_name() << "\n";

		// extract the block name from the full path			
		std::string full = iter->path().string();
		// get rid of the root
		std::string block_name = full.substr(current_dir.string().size());
		// get rid of the extension
		size_t pos = block_name.find_last_of('.');
		block_name = block_name.substr(0, pos);
		std::cout << block_name;
		break;
	}
}

TEST(LibTest, testBlock)
{
	std::ifstream ifs(kDataPath + "blocks\\行业\\交通工具.txt");
	std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	int i;
	i = json.size();
	std::cout << json << std::endl;
}

TEST(UtilTest, testJsonParser)
{
	prism::JsonDoc doc;
	bool ret = prism::ParseJson(kDataPath + "strategy.txt", doc);
	
	assert(doc.HasMember("version"));
	assert(doc["version"].IsString());
	printf("version = %s\n", doc["version"].GetString());

	assert(doc.HasMember("stocks"));
	assert(doc["stocks"].IsString());
	printf("stocks = %s\n", doc["stocks"].GetString());

	assert(doc.HasMember("screen_rule"));
	prism::JsonValue& screen_rule = doc["screen_rule"];
	assert(screen_rule.IsObject());

	printf("screen_rule.type = %s\n", screen_rule["type"].GetString());
	assert(screen_rule.HasMember("rules"));
	assert(screen_rule["rules"].IsArray());

}

TEST(UtilTest, testJsonArchive)
{
	prism::JsonArchive ar(kDataPath + "Jsontest.txt");
	EXPECT_TRUE(ar.Open());
	prism::JsonSerializer* writer = ar.GetSerializer();
	writer->StartObject();
	writer->String("age");
	writer->Int(12);
	writer->String("school");
	writer->String("zju");
	writer->String("GPA");
	writer->Double(99.99);
	writer->EndObject();
	ar.Close();
}

TEST(LibTest, testRapidJson)
{
	////////////////////////////////////////////////////////////////////////////
	// 1. Parse a JSON text string to a document.

	const char json[] = " { \"obj\": {\"p1\": \"v1\", \"p2\": 4}, \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
	printf("Original JSON:\n %s\n", json);

	Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

#if 0
	// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
	document.Parse<0>(json);
		
#else
	// In-situ parsing, decode strings directly in the source string. Source must be string.
	char buffer[sizeof(json)];
	memcpy(buffer, json, sizeof(json));
	document.ParseInsitu<0>(buffer);
		
#endif

	printf("\nParsing to document succeeded.\n");

	////////////////////////////////////////////////////////////////////////////
	// 2. Access values in document. 

	printf("\nAccess values in document:\n");
	assert(document.IsObject());	// Document is a JSON value represents the root of DOM. Root can be either an object or array.

	assert(document.HasMember("hello"));
	assert(document["hello"].IsString());
	printf("hello = %s\n", document["hello"].GetString());

	assert(document["t"].IsBool());		// JSON true/false are bool. Can also uses more specific function IsTrue().
	printf("t = %s\n", document["t"].GetBool() ? "true" : "false");

	assert(document["f"].IsBool());
	printf("f = %s\n", document["f"].GetBool() ? "true" : "false");

	printf("n = %s\n", document["n"].IsNull() ? "null" : "?");

	assert(document["i"].IsNumber());	// Number is a JSON type, but C++ needs more specific type.
	assert(document["i"].IsInt());		// In this case, IsUint()/IsInt64()/IsUInt64() also return true.
	printf("i = %d\n", document["i"].GetInt());	// Alternative (int)document["i"]

	assert(document["pi"].IsNumber());
	assert(document["pi"].IsDouble());
	printf("pi = %g\n", document["pi"].GetDouble());

	const Value& obj = document["obj"];
	assert(obj.IsObject());
	printf("obj.p1 = %s\n", document["obj"]["p1"].GetString());
	printf("obj.p2 = %d\n", document["obj"]["p2"].GetInt());


	const Value& a = document["a"];	// Using a reference for consecutive access is handy and faster.
	assert(a.IsArray());
	for (SizeType i = 0; i < a.Size(); i++)	// rapidjson uses SizeType instead of size_t.
		printf("a[%d] = %d\n", i, a[i].GetInt());

	// Note:
	//int x = a[0].GetInt();					// Error: operator[ is ambiguous, as 0 also mean a null pointer of const char* type.
	int y = a[SizeType(0)].GetInt();			// Cast to SizeType will work.
	int z = a[0u].GetInt();						// This works too.
	(void)y;
	(void)z;

}
