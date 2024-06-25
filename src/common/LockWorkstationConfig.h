/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _LW_COMMON_DEFS_H_
#define _LW_COMMON_DEFS_H_

#include <ObjectList.h>
#include <GraphicsDefs.h>
#include <Message.h>
#include <String.h>

#define mPathToConfigFile                "LockWorkstationSettings"

#define mNameConfigExecDir               "execDir"
#define mNameConfigAuthMode              "authMode"
#define mNameConfigUser                  "username"
#define mNameConfigPass                  "password"
#define mNameConfigBgMode                "bgMode"
#define mNameConfigBgColor               "bgColor"
#define mNameConfigImagePath             "imagePath"
#define mNameConfigImageList             "imageList"
#define mNameConfigImageFile             "imageStatic"
#define mNameConfigBoolClock 		     "clockShown"
#define mNameConfigClockColor            "clockColor"
#define mNameConfigClockFontSize 	     "fontSize"
#define mNameConfigClockPlace            "clockPos"
#define mNameConfigLanguage 			 "language"
#define mNameConfigBgSnooze              "bgSnooze"
#define mNameConfigSessionBarOn          "_sessionbar_visible"
#define mNameConfigSysInfoPanelOn        "_sysinfopanel_visible"
#define mNameConfigKillerShortcutOn      "_shortcuts_killer"
#define mNameConfigEvtLoggingOn          "_logging_enabled"

void        DefaultSettings(BMessage* archive);
status_t    LoadSettings(BMessage* archive);
status_t    SaveSettings(BMessage* archive);

enum BgMode {
    BGM_NONE = 0,
    BGM_STATIC,
    BGM_FOLDER,
    BGM_LISTFILE
};
enum AuthMethod {
    AUTH_SYSTEM_ACCOUNT = 0,
    AUTH_KEYSTORE,
    AUTH_APP_ACCOUNT
};

class LWSettings
{
public:
                LWSettings(const char* filename);

    static void DefaultSettings(BMessage* archive);
    status_t    SaveSettings();
    void        Commit();
    void        Reset();
public:
    AuthMethod  AuthenticationMethod();
    const char* DefaultUser();
    const char* DefaultUserPassword();
    BgMode      BackgroundMode();
    rgb_color   BackgroundColor();
    const char* BackgroundImageFolderPath();
    const char* BackgroundImageListPath();
    const char* BackgroundImageStaticPath();
    uint32      BackgroundImageSnooze();
    bool        ClockIsEnabled();
    rgb_color   ClockColor();
    BPoint      ClockLocation();
    uint32      ClockSize();
    bool        SessionBarIsEnabled();
    bool        SystemInfoPanelIsEnabled();
    bool        KillerShortcutIsEnabled();
    bool        EventLogIsEnabled();
public:
    status_t    SetAuthenticationMethod(AuthMethod method);
    status_t    SetDefaultUser(const char* newname);
    status_t    SetDefaultUserPassword(const char* newpass);
    status_t    SetBackgroundMode(BgMode mode);
    status_t    SetBackgroundColor(rgb_color color);
    status_t    SetBackgroundImageFolderPath(BString path);
    status_t    SetBackgroundImageListPath(BString path);
    status_t    SetBackgroundImageStatic(BString path);
    status_t    SetBackgroundImageSnooze(uint32 multiplier);
    status_t    SetClockEnabled(bool status);
    status_t    SetClockColor(rgb_color color);
    status_t    SetClockLocation(BPoint point);
    status_t    SetClockSize(uint32 fontsize);
    status_t    SetSessionBarEnabled(bool status);
    status_t    SetSystemInfoPanelEnabled(bool status);
    status_t    SetKillerShortcutEnabled(bool status);
    status_t    SetEventLogEnabled(bool status);
private:
	status_t	LoadSettings();
	void		InitData();
private:
	BString     filepath;
	BMessage	savemessage;
    AuthMethod  fAuthMethod;
    BString		mStringUser1;
    BString		mStringPassword1;
    BgMode      fBackgroundMode;
    rgb_color   fBackgroundColor;
    BString     fBackgroundImageFolder;
    BString     fBackgroundImageListFile;
    BString     fBackgroundImageStatic;
    uint32      fBackgroundImageSnooze;
    bool        fClockEnabled;
    rgb_color   fClockColor;
    uint32      fClockSize;
    BPoint      fClockLocation;
    BString		fLanguage;
    bool        fSessionBarEnabled;
    bool        fSystemInfoPanelEnabled;
    bool        fKillerShortcutEnabled;
    bool        fEventLogEnabled;
};

#endif /* _LW_COMMON_DEFS_H_ */
