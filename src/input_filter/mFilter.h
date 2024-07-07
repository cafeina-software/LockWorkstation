/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mFilter_h_
#define _mFilter_h_

#include <Application.h>
#include <Locker.h>
#include <MessageFilter.h>
#include <SupportDefs.h>
#include <cstdio>
#include <InputServerFilter.h>
#include "mFilterMsgSvr.h"

extern "C" BInputServerFilter* instantiate_input_filter();

class LockWorkstationInputFilter : public BInputServerFilter, private BLocker
{
public:
                            LockWorkstationInputFilter();
    virtual                ~LockWorkstationInputFilter();

    virtual status_t        InitCheck();
    virtual filter_result   Filter(BMessage* message, BList* outList);
    virtual status_t        GetScreenRegion(BRegion* region) const;

    void                    Activate(bool value);
    bool                    IsActivated();
private:
    mFilterMsgSvr*          fLooper;
    BMessageRunner*         fMsgRunner;
    bool                    fActive;
};

#endif /* _mFilter_h_ */
