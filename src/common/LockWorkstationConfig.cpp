/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <File.h>
#include <FindDirectory.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Path.h>
#include <Roster.h>
#include <SupportDefs.h>
#include <cstdio>

#include "LockWorkstationConfig.h"

void DefaultSettings(BMessage* archive)
{
    archive->MakeEmpty();

    archive->AddString(mNameConfigUser, "baron");
    archive->AddString(mNameConfigPass, "haikubox");

    archive->AddUInt8(mNameConfigBgMode, 1);
    archive->AddColor(mNameConfigBgColor, {0, 0, 0, 255});
    BRoster roster;
    entry_ref appref;
    roster.FindApp("application/x-vnd.LockWorkstation", &appref);
    BEntry appentry(&appref);
    BPath apppath;
    appentry.GetPath(&apppath);
    BPath defpath;
    apppath.GetParent(&defpath);
    defpath.Append("images/default", true);
    archive->AddString(mNameConfigImagePath, defpath.Path());
    archive->AddString(mNameConfigImageList, NULL);
    archive->AddUInt32(mNameConfigBgSnooze, 10);

    archive->AddColor(mNameConfigClockColor, {128, 0, 0, 255});
    archive->AddPoint(mNameConfigClockPlace, BPoint(0, 0));
    archive->AddBool(mNameConfigBoolClock, false);
    archive->AddUInt32(mNameConfigClockFontSize, 8);

    archive->AddString(mNameConfigLanguage, "0");

    archive->AddBool(mNameConfigSessionBarOn, false);
    archive->AddBool(mNameConfigSysInfoPanelOn, true);
    archive->AddBool(mNameConfigKillerShortcutOn, false);
    archive->AddBool(mNameConfigEvtLoggingOn, true);
}

status_t LoadSettings(BMessage* archive)
{
    status_t status = B_OK;
    
    BPath path;
    find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(mPathToConfigFile);

    BFile file;
    status = file.SetTo(path.Path(), B_READ_WRITE);
    switch(status)
	{
		case B_OK:
            fprintf(stderr, "Read successfully...\n");
			archive->Unflatten(&file);
			break;
		case B_BAD_VALUE:
		case B_ENTRY_NOT_FOUND:
		{
			// If there is no settings, let's create one with the default values
            fprintf(stderr, "No settings file found, creating one...\n");
			DefaultSettings(archive);
			if((status = SaveSettings(archive)) != B_OK)
				return status;
			archive->Unflatten(&file);
			break;
		}
		case B_PERMISSION_DENIED:
		{
			// If permission was denied, we could (only) use
			//   a temporary setting until the issue is solved:
			//   the hardcoded default (or offer the option
			//   to save them in another location if implemented...)
            fprintf(stderr, "Permission denied...\n");
			DefaultSettings(archive);
			break;
		}
		case B_NO_MEMORY:
		case B_FILE_ERROR:
		case B_NO_MORE_FDS:
		case B_LINK_LIMIT:
		default:
			// Something went wrong, let's end with this
			//   because we don't have the needed data
            fprintf(stderr, "Bad error...\n");
			// be_app->PostMessage(B_QUIT_REQUESTED);
			return status;
	}

    return status;
}

status_t SaveSettings(BMessage* archive)
{
    status_t status = B_ERROR;
    BPath path;

    find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(mPathToConfigFile);

    BFile file;
    if((status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE)) != B_OK)
        return status;

    archive->Flatten(&file);

    return B_OK;
}
