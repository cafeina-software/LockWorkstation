/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <InputServerFilter.h>
#include <Looper.h>
#include <Roster.h>
#include <String.h>
#include <cstdio>
#include "mFilter.h"
#include "mFilterMsgSvr.h"

mFilterMsgSvr::mFilterMsgSvr(BInputServerFilter* owner, const char* name,
    int32 priority, int32 ports)
: BLooper(name, priority, ports),
  fOwnerFilter(owner)
{
}

void mFilterMsgSvr::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
        case 'test':
            ((LockWorkstationInputFilter*)fOwnerFilter)->Activate(be_roster->IsRunning("application/x-vnd.LockWorkstation"));
            break;
		case B_SOME_APP_LAUNCHED:
        {
			dprintf(STDERR_FILENO, "Launched!!!\n");

            BString signature;
            if(message->FindString("be:signature", &signature) == B_OK &&
            strcmp("application/x-vnd.LockWorkstation", signature.String()) == 0)
                ((LockWorkstationInputFilter*)fOwnerFilter)->Activate(true);
			break;
        }
		case B_SOME_APP_QUIT:
        {
			printf("Quitted!!!\n");

            BString signature;
            if(message->FindString("be:signature", &signature) == B_OK &&
            strcmp("application/x-vnd.LockWorkstation", signature.String()) == 0)
                ((LockWorkstationInputFilter*)fOwnerFilter)->Activate(false);
			break;
        }
		default:
			BLooper::MessageReceived(message);
	}
}
