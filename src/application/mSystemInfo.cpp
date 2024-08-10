/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <OS.h>
#include <sys/utsname.h>
#include <cpu_type.h>
#include "mSystemInfo.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "System info"

mSystemInfo::mSystemInfo(BRect frame)
: BView(frame, NULL, B_FOLLOW_NONE, B_WILL_DRAW | B_PULSE_NEEDED)
{
    InitUIData();

    SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    infoFont.SetFace(B_BOLD_FACE);
    infoFont.SetFlags(B_FORCE_ANTIALIASING);
    infoFont.SetSize(be_bold_font->Size() * 1.5);

    subInfoFont.SetFace(B_REGULAR_FACE);
    subInfoFont.SetFlags(B_FORCE_ANTIALIASING);
    subInfoFont.SetSize(be_plain_font->Size() * 1.25);

    BStringView* ostitle = AddTitle(B_TRANSLATE("Operating system"));
    BStringView* osname = AddText(fStrNameAndRelease);
    BStringView* oshrev = AddText(fStrHrev);

    BStringView* cputitle = AddTitle(B_TRANSLATE("Processor"));
    BStringView* cpuname = AddText(fStrFullCpuName);
    BStringView* cpucores = AddText(fStrFullCoreInfo);

    BStringView* memtitle = AddTitle(B_TRANSLATE("Memory"));
    memmax = AddText(fStrMaxMem);
    memused = AddText(fStrUsedMem);

    BStringView* sessiontitle = AddTitle(B_TRANSLATE("Session"));
    BStringView* hostname = AddText(fStrNodeName);
    runtime = AddText(fStrRunningTime);

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .SetInsets(B_USE_WINDOW_INSETS)
            .Add(ostitle)
            .Add(osname)
            .Add(oshrev)
            .AddStrut(12.0f)
            .Add(cputitle)
            .Add(cpuname)
            .Add(cpucores)
            .AddStrut(12.0f)
            .Add(memtitle)
            .Add(memmax)
            .Add(memused)
            .AddStrut(12.0f)
            .Add(sessiontitle)
            .Add(hostname)
            .Add(runtime)
    .End();

    ResizeToPreferred();
}

mSystemInfo::~mSystemInfo()
{
}

void mSystemInfo::Pulse()
{
    ThreadedCall(thRunningTime, UpdateUIThreadCall, "Update UI",
        B_DISPLAY_PRIORITY, this);
}

void mSystemInfo::AttachedToWindow()
{
    BView::AttachedToWindow();
}

void mSystemInfo::Draw(BRect updateRect)
{
    BView::Draw(updateRect);

    Invalidate();
}

void mSystemInfo::InitUIData()
{
    utsname uts;
    uname(&uts);
    fStrNameAndRelease = BString(ParseSystemName(&uts));
    fStrHrev = BString(ParseSystemHrev(&uts));
    fStrNodeName = BString(ParseNodeName(&uts));

    system_info info;
    get_system_info(&info);
    fStrMaxMem = BString(ParseMaxMem(&info));
    fStrUsedMem = BString(ParseUsedMem(&info));

    uint32 nodecount = 0;
    get_cpu_topology_info(NULL, &nodecount);
    cpu_topology_node_info *cputnik = new cpu_topology_node_info[nodecount];
    get_cpu_topology_info(cputnik, &nodecount);
    fStrFullCpuName = BString(ParseCPUInfo(cputnik, nodecount));
    fStrFullCoreInfo = BString(ParseCPUCores(cputnik, nodecount));

    fStrRunningTime = BString(ParseTime(system_time()));

    delete[] cputnik;
}

void mSystemInfo::Update()
{
    system_info info;

    get_system_info(&info);
    fStrRunningTime = BString(ParseTime(system_time()));
    fStrMaxMem = BString(ParseMaxMem(&info));
    fStrUsedMem = BString(ParseUsedMem(&info));

    LockLooper();

    memmax->SetText(fStrMaxMem.String());
    memused->SetText(fStrUsedMem.String());
    runtime->SetText(fStrRunningTime.String());

    UnlockLooper();
    LockLooper();
    UnlockLooper();
}

BStringView* mSystemInfo::AddTitle(const char* title)
{
    BStringView* titleView = new BStringView(title, title);
    titleView->SetFont(&infoFont);
    titleView->SetHighUIColor(B_LINK_TEXT_COLOR);

    return titleView;
}

BStringView* mSystemInfo::AddText(const char* text)
{
    BStringView* textView = new BStringView(text, text);
    textView->SetFont(&subInfoFont);
    textView->SetHighUIColor(B_WINDOW_TEXT_COLOR);

    return textView;
}

int32 mSystemInfo::UpdateUIThreadCall(void* data)
{
    mSystemInfo* target = (mSystemInfo*)data;
    target->Update();
    return 0;
}

const char*	mSystemInfo::ParseSystemName(utsname* uts)
{
    BString systemname("");
    // uname(uts);
    systemname << uts->sysname << " R" << uts->release << " " << uts->machine;
    fprintf(stderr, "Result: %s\n", systemname.String());
    return systemname.String();
}

const char* mSystemInfo::ParseSystemHrev(utsname* uts)
{
    BString hrev(uts->version);
    return hrev.String();
}

const char* mSystemInfo::ParseTime(bigtime_t posixtime)
{
    uint32 seconds = round(posixtime / 1000000);
    uint32 minutes = 0, hours = 0, days = 0;

    while(seconds >= 60) {
        seconds -= 60;
        minutes++;
        if(minutes >= 60) {
            minutes -= 60;
            hours++;
            if(hours >= 24) {
                hours -= 24;
                days++;
            }
        }
    }

    BString str1;
    str1.SetToFormat(B_TRANSLATE("%d days, "), days);

    BString str2;
    str2.SetToFormat(B_TRANSLATE_COMMENT("Running time: %s%d hours, %d minutes, %d seconds",
        "Example: \"Running time: (2 days, )11 hours, 23 minutes, 12 seconds.\""),
        days > 0 ? str1.String() : "", hours, minutes, seconds);

    return str2.String();
}

const char* mSystemInfo::ParseCPUInfo(cpu_topology_node_info* cputnik, uint32 nodecount)
{
    BString cpuname;
    cpu_platform platform;
    cpu_vendor vendor;
    uint32 model;
    uint64 freq;

    for(int i = 0; i < nodecount; i++) {
        switch(cputnik[i].type)
        {
            case B_TOPOLOGY_ROOT:
                platform = cputnik[i].data.root.platform;
                break;
            case B_TOPOLOGY_CORE:
                model = cputnik[i].data.core.model;
                freq = cputnik[i].data.core.default_frequency;
                break;
            case B_TOPOLOGY_PACKAGE:
                vendor = cputnik[i].data.package.vendor;
                break;
            case B_TOPOLOGY_SMT:
            case B_TOPOLOGY_UNKNOWN:
            default:
                break;
        }
    }

    cpuname.SetToFormat(B_TRANSLATE("%s %s @ %.2lf GHz"),
        get_cpu_vendor_string(vendor),
        get_cpu_model_string(platform, vendor, model),
        freq / pow(1000.0f, 3));

    return cpuname.String();
}

const char* mSystemInfo::ParseCPUCores(cpu_topology_node_info* cputnik, uint32 nodecount)
{
    BString coreinfo;
    uint32 corecount = 0;

    for(int i = 0; i < nodecount; i++) {
        if(cputnik[i].type == B_TOPOLOGY_CORE)
            corecount++;
        else
            continue;
    }

    coreinfo.SetToFormat(B_TRANSLATE("%u cores"), corecount);

    return coreinfo.String();
}

const char* mSystemInfo::ParseMaxMem(system_info* info)
{
    BString maxmem;
    uint64 total = 0;

    total += (info->max_pages * B_PAGE_SIZE);
    total += (info->ignored_pages * B_PAGE_SIZE);
    maxmem.SetToFormat(B_TRANSLATE("Total memory: %.2lf GiB"),
        (total / pow(1024.0f, 3)));

    return maxmem.String();
}

const char* mSystemInfo::ParseUsedMem(system_info* info)
{
    BString usedmem;

    usedmem.SetToFormat(B_TRANSLATE("Used memory: %.2lf GiB"),
        ((info->used_pages * B_PAGE_SIZE) / pow(1024.0f, 3)));

    return usedmem.String();
}

const char*	mSystemInfo::ParseNodeName(utsname* uts)
{
    BString nodename;

    nodename.SetToFormat(B_TRANSLATE("Host name: %s"), uts->nodename);

    return nodename.String();
}

