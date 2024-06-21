/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _LW_COMMON_DEFS_H_
#define _LW_COMMON_DEFS_H_

#include <Message.h>

#define mPathToConfigFile                "LockWorkstationSettings"

#define mNameConfigExecDir               "execDir"
#define mNameConfigUser                  "username"
#define mNameConfigPass                  "password"
#define mNameConfigBgMode                "bgMode"
#define mNameConfigBgColor               "bgColor"
#define mNameConfigImagePath             "imagePath"
#define mNameConfigImageList             "imageList"
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

#endif /* _LW_COMMON_DEFS_H_ */
