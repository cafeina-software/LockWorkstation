/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mLogger_h_
#define _mLogger_h_

#include <DateTime.h>
#include <SupportDefs.h>
#include <File.h>
#include "../common/LockWorkstationConfig.h"

class mLogger
{
public:
                        mLogger(LWSettings* settings, const char* logfile);
	status_t            AddEvent(const char* desc);
    status_t            AddEvent(EventLevel level, const char* desc);
	status_t            AddEvent(BDateTime datetime, EventLevel level, const char* desc);
	status_t            Clear();
private:
	status_t            _CreateLogFile(bool enabled, const char* filename);
    const char*         _LevelToString(EventLevel level);
private:
    const char         *filename;
    LWSettings         *fCurrentSettings;
    const bool          fIsEnabled;
    const EventLevel    fLevel;
    const LogRetPolicy  fRetentionPolicy;
    const uint32        fMaxSize;
    const uint32        fMaxAge;
};


#endif /* _mLogger_h_ */
