// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@gmail.com (Ray Ni)
//
// routines and classes for action, which will be taken during strategy execution

#ifndef ACTION_H
#define ACTION_H

#include "common.h"
#include <string>
#include <vector>

namespace prism {

	class IAction
	{
	public:
		virtual ~IAction(){}
		virtual void Do() = 0;
	};

}

#endif