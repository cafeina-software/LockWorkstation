/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mThreaded_h_
#define _mThreaded_h_

#include <OS.h>
#include <SupportDefs.h>

class ThreadedClass
{
public:
	void            ThreadedCall(thread_id thid, int (*func)(void*),
                        const char* name, int32 priority, void* data);
};

#endif /* _mThreaded_h_ */
