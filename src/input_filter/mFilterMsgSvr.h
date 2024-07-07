/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mFilterMsgSvr_h_
#define _mFilterMsgSvr_h_

#include <InputServerFilter.h>
#include <Looper.h>
#include <SupportDefs.h>

class mFilterMsgSvr : public BLooper
{
public:
                    mFilterMsgSvr(BInputServerFilter* owner, const char* name,
                        int32 priority = B_NORMAL_PRIORITY,
                        int32 port = B_LOOPER_PORT_DEFAULT_CAPACITY);
	virtual void    MessageReceived(BMessage* message);
private:
    BInputServerFilter* fOwnerFilter;
};


#endif /* _mFilterMsgSvr_h_ */
