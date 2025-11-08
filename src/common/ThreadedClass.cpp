/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "ThreadedClass.h"

thread_id ThreadedClass::ThreadedCall(thread_id thid, int32 (*func)(void*),
	const char* name, int32 priority, void* data)
{
	thread_id th = spawn_thread(func, name, priority, data);
    resume_thread(th);
    return th;
}
