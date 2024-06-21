/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "ThreadedClass.h"

void ThreadedClass::ThreadedCall(thread_id thid, int (*func)(void*),
	const char* name, int32 priority, void* data)
{
	thid = spawn_thread(func, name, priority, data);
    resume_thread(thid);
}
