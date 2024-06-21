/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mLogger_h_
#define _mLogger_h_

#include <DateTime.h>
#include <SupportDefs.h>
#include <File.h>

enum EventLevel {
    EVT_CRITICAL,
    EVT_ERROR,
    EVT_WARNING,
    EVT_INFO,
    EVT_NONE
};

class mLogger
{
public:
	mLogger(bool enabled, const char* logfile);
	status_t AddEvent(const char* desc);
    status_t AddEvent(EventLevel level, const char* desc);
	status_t AddEvent(BDateTime datetime, EventLevel level, const char* desc);
	status_t Clear();
private:
	status_t _CreateLogFile(bool enabled, const char* filename);
    const char* _LevelToString(EventLevel level);
private:
    const char* filename;
    bool isenabled;
};


#endif /* _mLogger_h_ */
