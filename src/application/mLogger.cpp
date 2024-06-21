/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "mLogger.h"
#include <Entry.h>
#include <File.h>
#include <cstdio>
#include <fstream>

mLogger::mLogger(bool enabled, const char* logfile)
: filename(logfile), isenabled(enabled)
{
	if(_CreateLogFile(isenabled, filename) != B_OK)
        fprintf(stderr, "Error creating log\n");
}

// #pragma mark -

status_t mLogger::AddEvent(const char* desc)
{
    return AddEvent(BDateTime::CurrentDateTime(B_LOCAL_TIME), EVT_INFO, desc);
}

status_t mLogger::AddEvent(EventLevel level, const char* desc)
{
    return AddEvent(BDateTime::CurrentDateTime(B_LOCAL_TIME), level, desc);
}

status_t mLogger::AddEvent(BDateTime datetime, EventLevel level, const char* desc)
{
    BString month, day, hour, minute, second;
    month << (datetime.Date().Month() < 10 ? "0" : "") << datetime.Date().Month();
    day << (datetime.Date().Day() < 10 ? "0" : "") << datetime.Date().Day();
    hour << (datetime.Time().Hour() < 10 ? "0" : "") << datetime.Time().Hour();
    minute << (datetime.Time().Minute() < 10 ? "0" : "") << datetime.Time().Minute();
    second << (datetime.Time().Second() < 10 ? "0" : "") << datetime.Time().Second();

    BString log;
    log << datetime.Date().Year() << "-" << month << "-" << day << " "
        << hour << ":" << minute << ":" << second << " ["
        << _LevelToString(level) << "] " << desc << "\n";

    if(this->isenabled) {
        std::fstream f(filename, std::ios::app | std::ios::out);
        f.write(log.String(), log.Length());
    }
    // else
        // fprintf(stderr, "%s", log.String());

    return B_OK;
}

status_t mLogger::Clear()
{
    BEntry entry(filename);
    if(entry.Remove() != B_OK)
        return B_ERROR;

    return _CreateLogFile(this->isenabled, filename);
}

// #pragma mark -

status_t mLogger::_CreateLogFile(bool enabled, const char* filename)
{
    if(enabled) {
        std::fstream file(filename, std::ios::app | std::ios::out);
        file.write("", 0);
    }

    return B_OK;
}

const char* mLogger::_LevelToString(EventLevel level)
{
    switch(level)
    {
        case EVT_CRITICAL:
            return "CRITICAL";
        case EVT_ERROR:
            return "ERROR";
        case EVT_WARNING:
            return "WARNING";
        case EVT_INFO:
            return "INFO";
        case EVT_NONE:
        default:
            return "UNKNOWN";
    }
}
