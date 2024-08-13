/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "mLogger.h"
#include <Entry.h>
#include <File.h>
#include <cstdio>
#include <ctime>
#include <fstream>

mLogger::mLogger(LWSettings* settings, const char* logfile)
: filename(logfile),
  fCurrentSettings(settings),
  fIsEnabled(settings->EventLogIsEnabled()),
  fLevel(static_cast<EventLevel>(settings->EventLogLevel())),
  fRetentionPolicy(static_cast<LogRetPolicy>(settings->EventLogRetentionPolicy())),
  fMaxSize(settings->EventLogMaxSize()),
  fMaxAge(settings->EventLogMaxAge())
{
    if(fIsEnabled) {
        BEntry entry(filename);
        if(!entry.Exists()) {
            if(_CreateLogFile(fIsEnabled, filename) != B_OK)
                fprintf(stderr, "Error creating log\n");
        }
        else {
            switch(fRetentionPolicy) {
                case EVP_WIPE_AFTER_SIZE:
                {
                    off_t size;
                    BFile file(filename, B_READ_ONLY);
                    file.GetSize(&size);
                    if(size > fMaxSize * 1024 * 1024) {
                        Clear();
                    }
                    break;
                }
                case EVP_WIPE_AFTER_AGE:
                {
                    time_t lasttime;
                    BFile file(&entry, B_READ_ONLY);
                    file.GetModificationTime(&lasttime);
                    time_t now = std::time(nullptr);

                    if(now > lasttime + (fMaxAge * 24 * 60 * 60)) {
                        Clear();
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
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

    if(fIsEnabled && level <= fLevel) {
        std::fstream f(filename, std::ios::app | std::ios::out);
        f.write(log.String(), log.Length());
    }

    return B_OK;
}

status_t mLogger::Clear()
{
    BEntry entry(filename);
    if(entry.Remove() != B_OK)
        return B_ERROR;

    return _CreateLogFile(fIsEnabled, filename);
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
