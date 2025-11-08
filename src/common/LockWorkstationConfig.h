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
#include "LockWorkstationCommonDefs.h"

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
#define mNameConfigEvtLoggingLevel       "_logging_level"
#define mNameConfigEvtLoggingRetention   "_logging_retention"
#define mNameConfigEvtLoggingMaxSize     "_logging_maxsize"
#define mNameConfigEvtLoggingMaxAge      "_logging_maxage"
#define mNameConfigPwdLessLogonOn        "_pwdless_enabled"
#define mNameConfigAuthAttemptsThrshld   "_auth_attempts"
#define mNameConfigAuthSnoozeAfterErrors "_auth_manyerrors_snz"
#define mNameConfigAuthResetForm         "_auth_form_reset"

class LWSettings
{
public:
                LWSettings(const char* filename);

    static void DefaultSettings(BMessage* archive);
    status_t    SaveSettings();
    void        Commit();
    bool        HasPendingData();
    void        Reset();
public:
    AuthMethod  AuthenticationMethod() const;
    const char* DefaultUser() const;
    const char* DefaultUserPassword() const;
    BgMode      BackgroundMode() const;
    rgb_color   BackgroundColor() const;
    const char* BackgroundImageFolderPath() const;
    const char* BackgroundImageListPath() const;
    const char* BackgroundImageStaticPath() const;
    uint32      BackgroundImageSnooze() const;
    ImgAdjust   BackgroundImageAdjustment() const;
    bool        ClockIsEnabled() const;
    rgb_color   ClockColor() const;
    BPoint      ClockLocation() const;
    uint32      ClockSize() const;
    bool        SessionBarIsEnabled() const;
    bool        SystemInfoPanelIsEnabled() const;
    bool        KillerShortcutIsEnabled() const;
    bool        EventLogIsEnabled();
    uint8       EventLogLevel();
    uint8       EventLogRetentionPolicy();
    uint32      EventLogMaxSize();
    uint32      EventLogMaxAge();
    bool        PasswordLessAuthEnabled() const;
    int32       AuthenticationAttemptsThreshold() const;
    int32       AuthenticationCooldownAfterThreshold() const;
    bool        AuthenticationResetFormIfInactive() const;
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
    status_t    SetEventLogLevel(uint8 level);
    status_t    SetEventLogRetentionPolicy(uint8 level);
    status_t    SetEventLogMaxSize(uint32 value);
    status_t    SetEventLogMaxAge(uint32 value);
    status_t    SetPasswordLessAuthEnabled(bool status);
    status_t    SetAuthenticationAttemptsThreshold(int32 count);
    status_t    SetAuthenticationCooldownAfterThreshold(int32 multiplier);
    status_t    SetAuthenticationResetFormIfInactive(bool status);
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
    uint8       fEventLogLevel;
    uint8       fEventLogRetentionPolicy;
    uint32      fEventLogMaxSize;
    uint32      fEventLogMaxAge;
    bool        fPasswordLessAuthEnabled;
    int32       fAuthAttemptsThreshold;
    int32       fAuthAttemptsErrorCooldown;
    bool        fAuthResetFormIfInactive;
};

#endif /* _LW_COMMON_DEFS_H_ */
