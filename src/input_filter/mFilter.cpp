/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <Alert.h>
#include <Autolock.h>
#include <KernelExport.h>
#include <MessageRunner.h>
#include <Roster.h>
#include <syslog.h>
#include "mFilter.h"
#include "mFilterMsgSvr.h"

#define trace(x, ...) syslog(LOG_INFO, "lockworkstation_filter [%s]: " x, __func__, ##__VA_ARGS__)

extern "C" _EXPORT BInputServerFilter* instantiate_input_filter()
{
    return new (std::nothrow) LockWorkstationInputFilter();
}

LockWorkstationInputFilter::LockWorkstationInputFilter()
: BInputServerFilter(),
  BLocker("LW_filter_locker"),
  fActive(false),
  fLooper(nullptr),
  fMsgRunner(nullptr)
{
    trace("created\n");

    fLooper = new (std::nothrow) mFilterMsgSvr(this, "LW_filter_looper");
    BAutolock lock(this);
    fLooper->Run();

    BMessage msg('test');
    fMsgRunner = new (std::nothrow) BMessageRunner(fLooper, &msg, 15000000);

    be_roster->StartWatching(fLooper);
}

LockWorkstationInputFilter::~LockWorkstationInputFilter()
{
    trace("destroying\n");

    be_roster->StopWatching(fLooper);
    if(fLooper && fLooper->Lock())
        fLooper->Quit();
    delete fMsgRunner;
}

status_t LockWorkstationInputFilter::InitCheck()
{
    trace("initcheck\n");
    return B_OK;
}

filter_result LockWorkstationInputFilter::Filter(BMessage* message, BList* outList)
{
    BAutolock lock(this);
    filter_result result = B_DISPATCH_MESSAGE;

    if(!IsActivated())
        return B_DISPATCH_MESSAGE;
    else {
        switch(message->what)
        {
            case 'fltr':
            {
                trace("filter status changed\n");
                Activate(message->GetBool("filter", false));
                break;
            }
            case B_UNMAPPED_KEY_DOWN:
            case B_UNMAPPED_KEY_UP:
            case B_KEY_DOWN:
            case B_KEY_UP:
            {
                int32 modifiers = message->GetInt32("modifiers", -1);
                int32 key = message->GetInt32("key", -1);

                if(be_roster->IsRunning("application/x-vnd.LockWorkstation")) {
                    if(modifiers & B_CONTROL_KEY && modifiers & B_COMMAND_KEY && key == 0x34) {
                        trace("lockworkstation_filter: detected and filtered!\n");
                        result = B_SKIP_MESSAGE;
                    }
                }

                break;
            }
        }
    }

    return result;
}

status_t LockWorkstationInputFilter::GetScreenRegion(BRegion* region) const
{
    trace("get screen region\n");
    return BInputServerFilter::GetScreenRegion(region);
}

void LockWorkstationInputFilter::Activate(bool value)
{
    BAutolock lock(this);
    fActive = value;
}

bool LockWorkstationInputFilter::IsActivated()
{
    return fActive;
}
