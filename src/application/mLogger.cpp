/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "mLogger.h"
#include "mConstant.h"
#include <Entry.h>
#include <File.h>
#include <cstdio>
#include <ctime>

mLogger::mLogger(LWSettings* settings, const char* logfile)
: BLooper("Logger looper", B_NORMAL_PRIORITY),
  fLogFilePath(logfile),
  fIsEnabled(settings->EventLogIsEnabled()),
  fLevel(static_cast<EventLevel>(settings->EventLogLevel())),
  fRetentionPolicy(static_cast<LogRetPolicy>(settings->EventLogRetentionPolicy())),
  fMaxSize(settings->EventLogMaxSize()),
  fMaxAge(settings->EventLogMaxAge())
{
    fLogFileEntry.SetTo(fLogFilePath.String(), false);

    if(fIsEnabled) {
        if(!fLogFileEntry.Exists()) {
            fLogFile.SetTo(&fLogFileEntry, B_WRITE_ONLY | B_CREATE_FILE | B_OPEN_AT_END);
            fLogFile.SetPermissions(S_IRUSR | S_IWUSR | S_IRGRP);
        }
        else {
            switch(fRetentionPolicy) {
                case EVP_WIPE_AFTER_SIZE:
                {
                    fLogFile.SetTo(&fLogFileEntry, B_READ_ONLY);
                    off_t size;
                    fLogFile.GetSize(&size);
                    if(size > fMaxSize * 1024 * 1024) {
                        Clear(); // There reopens in write mode
                    }
                    break;
                }
                case EVP_WIPE_AFTER_AGE:
                {
                    time_t lasttime;
                    fLogFile.SetTo(&fLogFileEntry, B_READ_ONLY);
                    fLogFile.GetModificationTime(&lasttime);
                    time_t now = std::time(nullptr);

                    if(now > lasttime + (fMaxAge * 24 * 60 * 60)) {
                        Clear(); // There reopens in write mode
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void mLogger::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case M_LOGGING_REQUESTED:
        {
            BDateTime datetime;

            const void* ptr = NULL;
            ssize_t length = 0;
            if(message->FindData("log_timestamp", B_TIME_TYPE, &ptr, &length) == B_OK) {
                uint8* buffer = new uint8[length];
                memcpy(buffer, ptr, length);
                time_t time = *(reinterpret_cast<time_t*>(buffer));
                datetime.SetTime_t(time);
                delete[] buffer;
            }
            else
                datetime.SetTime_t(std::time(NULL));

            _WriteLogEvent(datetime,
                static_cast<EventLevel>(message->GetInt32("log_level", EVT_INFO)),
                message->GetString("log_description"));

            break;
        }
        case M_LOGGING_CLEANUP:
            Clear();
            break;
        default:
            return BLooper::MessageReceived(message);
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
    if(!fIsEnabled || level > fLevel)
        return B_OK;

    BMessage request(M_LOGGING_REQUESTED);
    time_t time = datetime.Time_t();
    request.AddData("log_timestamp", B_TIME_TYPE, &time, sizeof(time));
    request.AddInt32("log_level", level);
    request.AddString("log_description", desc);

    return PostMessage(&request);
}

status_t mLogger::Clear()
{
    if(fLogFileEntry.Exists())
        if(fLogFileEntry.Remove() != B_OK)
            return B_ERROR;

    if(fIsEnabled) {
        fLogFile.SetTo(&fLogFileEntry, B_WRITE_ONLY | B_CREATE_FILE | B_OPEN_AT_END);
        fLogFile.SetPermissions(S_IRUSR | S_IWUSR | S_IRGRP);
    }

    return B_OK;
}

// #pragma mark -

status_t mLogger::_WriteLogEvent(BDateTime datetime, EventLevel level, const char* desc)
{
    if(fIsEnabled && level <= fLevel) {
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

        if(!fLogFile.IsWritable()) {
            fLogFile.SetTo(&fLogFileEntry, B_WRITE_ONLY | B_CREATE_FILE | B_OPEN_AT_END);
            if(fLogFile.InitCheck() != B_OK) {
                return B_ERROR;
            }
        }

        ssize_t writtenBytes = fLogFile.Write(log.String(), log.Length());
        if(writtenBytes < 0) {
            return B_IO_ERROR;
        }
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

void PostEventToLog(EventLevel level, const char* description)
{
    BMessage logRequest(M_LOGGING_REQUESTED);
    logRequest.AddUInt32("log_level", static_cast<uint32>(level));
    logRequest.AddString("log_description", description);
    be_app->PostMessage(&logRequest);
}
