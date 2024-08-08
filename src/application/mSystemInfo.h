/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef __M_SYSTEM_INFO_H
#define __M_SYSTEM_INFO_H

#include <InterfaceKit.h>
#include <SupportDefs.h>
#include <sys/utsname.h>
#include "mClockView.h"
#include "../common/ThreadedClass.h"

class mSystemInfo : public BView, public ThreadedClass
{
public:
                    mSystemInfo(BRect frame);
    virtual         ~mSystemInfo();
    virtual void    Pulse();
    virtual void    AttachedToWindow();
    virtual void    Draw(BRect updateRect);
    void            Update();
private:
    void            InitUIData();
    BStringView*    AddTitle(const char* title);
    BStringView*    AddText(const char* text);

	const char*		ParseSystemName(utsname*);
	const char*     ParseSystemHrev(utsname*);
    const char*     ParseTime(bigtime_t posixtime);
    const char* 	ParseCPUInfo(cpu_topology_node_info*, uint32 nodecount);
    const char* 	ParseCPUCores(cpu_topology_node_info*, uint32 nodecount);
    const char*     ParseMaxMem(system_info*);
    const char* 	ParseUsedMem(system_info*);
    const char*		ParseNodeName(utsname*);

    static int32    UpdateUIThreadCall(void* data);
private:
    mClockView*     clock;
    BStringView     *runtime,
                    *memmax,
                    *memused;

    BFont           infoFont;
    BFont           subInfoFont;

    BString         fStrNameAndRelease;
    BString			fStrHrev;
    BString         fStrFullCpuName;
    BString         fStrFullCoreInfo;
    BString         fStrMaxMem;
    BString         fStrUsedMem;
    BString         fStrNodeName;
    BString         fStrRunningTime;

    BString         sysname;
    BString         sysrelease;
    BString         syshrev;
    BString         sysnodename;
    BString         hwmachine;
    BString         hwmemused;
    BString         hwmemmax;
    BString         cpuplatform;
    BString         cpumodel;
    BString         cpufreq;

    thread_id       thRunningTime;
};


#endif /* __M_SYSTEM_INFO_H */
