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
#define mNameConfigImageAdjustment       "imageAdjust"
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
#define mNameConfigPwdLessLogonOn        "_pwdless_enabled"
#define mNameConfigAuthAttemptsThrshld   "_auth_attempts"
#define mNameConfigAuthSnoozeAfterErrors "_auth_manyerrors_snz"

enum BgMode {
    BGM_NONE = 0,
    BGM_STATIC,
    BGM_FOLDER,
    BGM_LISTFILE
};
enum ImgAdjust {
    BGI_ADJ_KEEP_AND_CENTER = 0,
    BGI_ADJ_SCALE_X = 1,
    BGI_ADJ_SCALE_Y = 2,
    BGI_ADJ_SCALE_X_Y = 3,
    BGI_ADJ_STRETCH_TO_SCREEN = 4
};
enum AuthMethod {
    AUTH_SYSTEM_ACCOUNT = 0,
    AUTH_APP_ACCOUNT = 2
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
    ImgAdjust   BackgroundImageAdjustment();
    bool        ClockIsEnabled();
    rgb_color   ClockColor();
    BPoint      ClockLocation();
    uint32      ClockSize();
    bool        SessionBarIsEnabled();
    bool        SystemInfoPanelIsEnabled();
    bool        KillerShortcutIsEnabled();
    bool        EventLogIsEnabled();
    bool        PasswordLessAuthEnabled();
    int32       AuthenticationAttemptsThreshold();
    int32       AuthenticationCooldownAfterThreshold();
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
    status_t    SetBackgroundImageAdjustment(ImgAdjust value);
    status_t    SetClockEnabled(bool status);
    status_t    SetClockColor(rgb_color color);
    status_t    SetClockLocation(BPoint point);
    status_t    SetClockSize(uint32 fontsize);
    status_t    SetSessionBarEnabled(bool status);
    status_t    SetSystemInfoPanelEnabled(bool status);
    status_t    SetKillerShortcutEnabled(bool status);
    status_t    SetEventLogEnabled(bool status);
    status_t    SetPasswordLessAuthEnabled(bool status);
    status_t    SetAuthenticationAttemptsThreshold(int32 count);
    status_t    SetAuthenticationCooldownAfterThreshold(int32 multiplier);
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
    ImgAdjust   fBackgroundImageAdjustment;
    bool        fClockEnabled;
    rgb_color   fClockColor;
    uint32      fClockSize;
    BPoint      fClockLocation;
    BString		fLanguage;
    bool        fSessionBarEnabled;
    bool        fSystemInfoPanelEnabled;
    bool        fKillerShortcutEnabled;
    bool        fEventLogEnabled;
    bool        fPasswordLessAuthEnabled;
    int32       fAuthAttemptsThreshold;
    int32       fAuthAttemptsErrorCooldown;
};

#endif /* _LW_COMMON_DEFS_H_ */
