/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef __mLockWorkstationCommonDefs_h__
#define __mLockWorkstationCommonDefs_h__

/* Authentication */
enum AuthMethod {
    AUTH_SYSTEM_ACCOUNT = 0,
    AUTH_APP_ACCOUNT = 2
};

/* Background */
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

/* Log */
enum EventLevel {
    EVT_CRITICAL,
    EVT_ERROR = 1,
    EVT_WARNING,
    EVT_INFO,
    EVT_NONE
};
enum LogRetPolicy {
    EVP_CONTINUE = 0,
    EVP_WIPE_AFTER_SIZE = 1,
    EVP_WIPE_AFTER_AGE = 2
};

#endif /* __mLockWorkstationCommonDefs_h__ */
