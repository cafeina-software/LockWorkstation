/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mSessionBar_h_
#define _mSessionBar_h_

#include <InterfaceKit.h>
#include <ToolBar.h>
#include <SupportDefs.h>

class mSessionBar : public BToolBar
{
public:
	mSessionBar(orientation ont = B_HORIZONTAL, BHandler* target = NULL);
private:
	BHandler* fTarget;
};


#endif /* _mSessionBar_h_ */
