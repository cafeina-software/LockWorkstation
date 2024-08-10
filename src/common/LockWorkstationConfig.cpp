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
#include <StringList.h>
#include <SupportDefs.h>
#include <cstdio>

#include "LockWorkstationConfig.h"

LWSettings::LWSettings(const char* filename)
{
    filepath.SetTo(filename);
    LoadSettings();
    InitData();

    fLanguage.SetTo("0");
}

status_t LWSettings::LoadSettings()
{
    BFile file;
    status_t status = file.SetTo(filepath, B_READ_WRITE);
    switch(status)
	{
		case B_OK:
            fprintf(stderr, "Read successfully...\n");
			savemessage.Unflatten(&file);
			break;
		case B_BAD_VALUE:
		case B_ENTRY_NOT_FOUND:
		{
			// If there is no settings, let's create one with the default values
            fprintf(stderr, "No settings file found, creating one...\n");
			LWSettings::DefaultSettings(&savemessage);
			if((status = SaveSettings()) != B_OK)
				return status;
			savemessage.Unflatten(&file);
			break;
		}
		case B_PERMISSION_DENIED:
		{
			// If permission was denied, we could (only) use
			//   a temporary setting until the issue is solved:
			//   the hardcoded default (or offer the option
			//   to save them in another location if implemented...)
            fprintf(stderr, "Permission denied...\n");
			LWSettings::DefaultSettings(&savemessage);
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
			return status;
	}

    return status;
}

void LWSettings::InitData()
{
    BMessage* defaults = new BMessage;
	DefaultSettings(defaults); // To be used when only some of the fields are missing

    fAuthMethod = static_cast<AuthMethod>(savemessage.GetUInt8(mNameConfigAuthMode,
        defaults->GetUInt8(mNameConfigAuthMode, 2)));
    fPasswordLessAuthEnabled = savemessage.GetBool(mNameConfigPwdLessLogonOn,
        defaults->GetBool(mNameConfigPwdLessLogonOn));
    fAuthAttemptsThreshold = savemessage.GetInt32(mNameConfigAuthAttemptsThrshld,
        defaults->GetInt32(mNameConfigAuthAttemptsThrshld, 0));
    fAuthAttemptsErrorCooldown = savemessage.GetInt32(mNameConfigAuthSnoozeAfterErrors,
        defaults->GetInt32(mNameConfigAuthSnoozeAfterErrors, 5));
	mStringUser1 = savemessage.GetString(mNameConfigUser,
		defaults->GetString(mNameConfigUser));
	mStringPassword1 = savemessage.GetString(mNameConfigPass,
		defaults->GetString(mNameConfigPass));

    fBackgroundMode = static_cast<BgMode>(savemessage.GetUInt8(mNameConfigBgMode,
        defaults->GetUInt8(mNameConfigBgMode, 1)));
	fBackgroundColor = savemessage.GetColor(mNameConfigBgColor,
        defaults->GetColor(mNameConfigBgColor, rgb_color()));
	fBackgroundImageFolder = savemessage.GetString(mNameConfigImagePath,
		defaults->GetString(mNameConfigImagePath, ""));
    fBackgroundImageListFile = savemessage.GetString(mNameConfigImageList,
        defaults->GetString(mNameConfigImageList, ""));
    fBackgroundImageStatic = savemessage.GetString(mNameConfigImageFile,
        defaults->GetString(mNameConfigImageFile, ""));
    fBackgroundImageSnooze = savemessage.GetUInt32(mNameConfigBgSnooze,
        defaults->GetUInt32(mNameConfigBgSnooze, 10));
    fBackgroundImageAdjustment = static_cast<ImgAdjust>(savemessage.GetUInt8(
        mNameConfigImageAdjustment, defaults->GetUInt8(mNameConfigImageAdjustment,
        BGI_ADJ_SCALE_X_Y)));

    fClockColor = savemessage.GetColor(mNameConfigClockColor,
        defaults->GetColor(mNameConfigClockColor, rgb_color()));
    fClockLocation = savemessage.GetPoint(mNameConfigClockPlace,
        defaults->GetPoint(mNameConfigClockPlace, BPoint()));
    fClockEnabled = savemessage.GetBool(mNameConfigBoolClock,
        defaults->GetBool(mNameConfigBoolClock));
    fClockSize = savemessage.GetUInt32(mNameConfigClockFontSize,
        defaults->GetUInt32(mNameConfigClockFontSize, 8));

    fSessionBarEnabled = savemessage.GetBool(mNameConfigSessionBarOn,
        defaults->GetBool(mNameConfigSessionBarOn));
    fSystemInfoPanelEnabled = savemessage.GetBool(mNameConfigSysInfoPanelOn,
        defaults->GetBool(mNameConfigSysInfoPanelOn));
    fKillerShortcutEnabled = savemessage.GetBool(mNameConfigKillerShortcutOn,
        defaults->GetBool(mNameConfigKillerShortcutOn));
    fEventLogEnabled = savemessage.GetBool(mNameConfigEvtLoggingOn,
        defaults->GetBool(mNameConfigEvtLoggingOn));

    delete defaults;
}

void LWSettings::DefaultSettings(BMessage* archive)
{
    archive->MakeEmpty();

    archive->AddUInt8(mNameConfigAuthMode, AUTH_APP_ACCOUNT);
    archive->AddBool(mNameConfigPwdLessLogonOn, true);
    archive->AddInt32(mNameConfigAuthAttemptsThrshld, 0);
    archive->AddInt32(mNameConfigAuthSnoozeAfterErrors, 5);
    archive->AddString(mNameConfigUser, "baron");
    archive->AddString(mNameConfigPass, "haikubox");

    archive->AddUInt8(mNameConfigBgMode, BGM_FOLDER);
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
    archive->AddString(mNameConfigImageList, "");
    archive->AddString(mNameConfigImageFile, "");
    archive->AddUInt32(mNameConfigBgSnooze, 10);
    archive->AddUInt8(mNameConfigImageAdjustment, BGI_ADJ_SCALE_X_Y);

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

status_t LWSettings::SaveSettings()
{
    status_t status = B_ERROR;
    BFile file;
    if((status = file.SetTo(filepath, B_WRITE_ONLY | B_CREATE_FILE)) != B_OK)
        return status;
    savemessage.Flatten(&file);

    return B_OK;
}

void LWSettings::Reset()
{
    LWSettings::DefaultSettings(&savemessage);
}

void LWSettings::Commit()
{
    /* Authentication */
    savemessage.SetUInt8(mNameConfigAuthMode, static_cast<uint8>(fAuthMethod));
    savemessage.SetBool(mNameConfigPwdLessLogonOn, fPasswordLessAuthEnabled);
    savemessage.SetInt32(mNameConfigAuthAttemptsThrshld, fAuthAttemptsThreshold);
    savemessage.SetInt32(mNameConfigAuthSnoozeAfterErrors, fAuthAttemptsErrorCooldown);

    /* User */
    savemessage.SetString(mNameConfigUser, mStringUser1);
    savemessage.SetString(mNameConfigPass, mStringPassword1);

    /* Background */
    savemessage.SetColor(mNameConfigBgColor, fBackgroundColor);
    savemessage.SetString(mNameConfigImagePath, fBackgroundImageFolder);
    savemessage.SetString(mNameConfigImageList, fBackgroundImageListFile);
    savemessage.SetString(mNameConfigImageFile, fBackgroundImageStatic);
    savemessage.SetUInt32(mNameConfigBgSnooze, fBackgroundImageSnooze);
    savemessage.SetUInt8(mNameConfigBgMode, fBackgroundMode);
    savemessage.SetUInt8(mNameConfigImageAdjustment, fBackgroundImageAdjustment);

    /* Clock */
    savemessage.SetColor(mNameConfigClockColor, fClockColor);
    savemessage.SetPoint(mNameConfigClockPlace, fClockLocation);
    savemessage.SetBool(mNameConfigBoolClock, fClockEnabled);
    savemessage.SetUInt32(mNameConfigClockFontSize, fClockSize);

	/* Language (unused) */
    savemessage.SetString(mNameConfigLanguage, fLanguage);

    /* Additional panels */
    savemessage.SetBool(mNameConfigSessionBarOn, fSessionBarEnabled);
    savemessage.SetBool(mNameConfigSysInfoPanelOn, fSystemInfoPanelEnabled);

    /* Other configs */
    savemessage.SetBool(mNameConfigKillerShortcutOn, fKillerShortcutEnabled);
    savemessage.SetBool(mNameConfigEvtLoggingOn, fEventLogEnabled);
}

// #pragma mark -

AuthMethod LWSettings::AuthenticationMethod()
{
    return fAuthMethod;
}

const char* LWSettings::DefaultUser()
{
    return mStringUser1.String();
}

const char* LWSettings::DefaultUserPassword()
{
    return mStringPassword1.String();
}

BgMode LWSettings::BackgroundMode()
{
    return fBackgroundMode;
}

rgb_color LWSettings::BackgroundColor()
{
    return fBackgroundColor;
}

const char* LWSettings::BackgroundImageFolderPath()
{
    return fBackgroundImageFolder.String();
}

const char* LWSettings::BackgroundImageListPath()
{
    return fBackgroundImageListFile.String();
}

const char* LWSettings::BackgroundImageStaticPath()
{
    return fBackgroundImageStatic.String();
}

uint32 LWSettings::BackgroundImageSnooze()
{
    return fBackgroundImageSnooze;
}

ImgAdjust LWSettings::BackgroundImageAdjustment()
{
    return fBackgroundImageAdjustment;
}

bool LWSettings::ClockIsEnabled()
{
    return fClockEnabled;
}

rgb_color LWSettings::ClockColor()
{
    return fClockColor;
}

BPoint LWSettings::ClockLocation()
{
    return fClockLocation;
}

uint32 LWSettings::ClockSize()
{
    return fClockSize;
}

bool LWSettings::SessionBarIsEnabled()
{
    return fSessionBarEnabled;
}

bool LWSettings::SystemInfoPanelIsEnabled()
{
    return fSystemInfoPanelEnabled;
}

bool LWSettings::KillerShortcutIsEnabled()
{
    return fKillerShortcutEnabled;
}

bool LWSettings::EventLogIsEnabled()
{
    return fEventLogEnabled;
}

bool LWSettings::PasswordLessAuthEnabled()
{
    return fPasswordLessAuthEnabled;
}

int32 LWSettings::AuthenticationAttemptsThreshold()
{
    return fAuthAttemptsThreshold;
}

int32 LWSettings::AuthenticationCooldownAfterThreshold()
{
    return fAuthAttemptsErrorCooldown;
}

// #pragma mark -

status_t LWSettings::SetAuthenticationMethod(AuthMethod method)
{
    return ((fAuthMethod  = method) == method) ? B_OK : B_ERROR;
}

status_t LWSettings::SetDefaultUser(const char* newname)
{
    mStringUser1.SetTo(newname);
    return B_OK;
}

status_t LWSettings::SetDefaultUserPassword(const char* newpass)
{
    mStringPassword1.SetTo(newpass);
    return B_OK;
}

status_t LWSettings::SetBackgroundMode(BgMode mode)
{
    fBackgroundMode = mode;
    return ((fBackgroundMode = mode) == mode) ? B_OK : B_ERROR;
}

status_t LWSettings::SetBackgroundColor(rgb_color color)
{
    return ((fBackgroundColor = color) == color) ? B_OK : B_ERROR;
}

status_t LWSettings::SetBackgroundImageFolderPath(BString path)
{
    fBackgroundImageFolder.SetTo(path.String());
    return B_OK;
}

status_t LWSettings::SetBackgroundImageListPath(BString path)
{
    fBackgroundImageListFile.SetTo(path.String());
    return B_OK;
}

status_t LWSettings::SetBackgroundImageStatic(BString path)
{
    fBackgroundImageStatic.SetTo(path.String());
    return B_OK;
}

status_t LWSettings::SetBackgroundImageSnooze(uint32 multiplier)
{
    return ((fBackgroundImageSnooze = multiplier) == multiplier) ? B_OK : B_ERROR;
}

status_t LWSettings::SetBackgroundImageAdjustment(ImgAdjust value)
{
    return ((fBackgroundImageAdjustment = value) == value) ? B_OK : B_ERROR;
}

status_t LWSettings::SetClockEnabled(bool status)
{
    return ((fClockEnabled = status) == status) ? B_OK : B_ERROR;
}

status_t LWSettings::SetClockColor(rgb_color color)
{
    return ((fClockColor = color) == color) ? B_OK : B_ERROR;
}

status_t LWSettings::SetClockLocation(BPoint point)
{
    return ((fClockLocation = point) == point) ? B_OK : B_ERROR;
}

status_t LWSettings::SetClockSize(uint32 fontsize)
{
    return ((fClockSize = fontsize) == fontsize) ? B_OK : B_ERROR;
}

status_t LWSettings::SetSessionBarEnabled(bool status)
{
    return ((fSessionBarEnabled = status) == status) ? B_OK : B_ERROR;
}

status_t LWSettings::SetSystemInfoPanelEnabled(bool status)
{
    return ((fSystemInfoPanelEnabled = status) == status) ? B_OK : B_ERROR;
}

status_t LWSettings::SetKillerShortcutEnabled(bool status)
{
    return ((fKillerShortcutEnabled = status) == status) ? B_OK : B_ERROR;
}

status_t LWSettings::SetEventLogEnabled(bool status)
{
    return ((fEventLogEnabled = status) == status) ? B_OK : B_ERROR;
}

status_t LWSettings::SetPasswordLessAuthEnabled(bool status)
{
    return ((fPasswordLessAuthEnabled = status) == status) ? B_OK : B_ERROR;
}

status_t LWSettings::SetAuthenticationAttemptsThreshold(int32 count)
{
    return ((fAuthAttemptsThreshold = count) == count) ? B_OK : B_ERROR;
}

status_t LWSettings::SetAuthenticationCooldownAfterThreshold(int32 multiplier)
{
    return ((fAuthAttemptsErrorCooldown = multiplier) == multiplier) ? B_OK : B_ERROR;
}
