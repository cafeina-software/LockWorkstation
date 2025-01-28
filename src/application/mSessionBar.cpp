/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <Catalog.h>
#include "mSessionBar.h"
#include "mConstant.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Session bar"

mSessionBar::mSessionBar(orientation ont, BHandler* target)
: BToolBar(ont), fTarget(target)
{
    SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    BString strRestart(B_TRANSLATE("Restart"));
    BString strShutdown(B_TRANSLATE("Shut down"));

    AddAction(M_RESTART_REQUESTED, this, NULL, strRestart, strRestart, false);
    AddAction(M_SHUTDOWN_REQUESTED, this, NULL, strShutdown, strShutdown, false);
}

void mSessionBar::AttachedToWindow()
{
    BToolBar::AttachedToWindow();

    for(const auto& command : { M_RESTART_REQUESTED, M_SHUTDOWN_REQUESTED })
        FindButton(command)->SetTarget(this);
}

void mSessionBar::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case M_RESTART_REQUESTED:
        case M_SHUTDOWN_REQUESTED:
            be_app->PostMessage(msg);
            break;
        default:
            BToolBar::MessageReceived(msg);
            break;
    }
}
