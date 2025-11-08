/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _M_LOGIN_BOX_H
#define _M_LOGIN_BOX_H

#include <InterfaceKit.h>
#include <SupportDefs.h>
#include "../common/LockWorkstationConfig.h"
#include "../common/ThreadedClass.h"

enum loginBoxMsgs {
    LBM_USERNAME_CHANGED = 'usch',
    LBM_PASSWORD_CHANGED = 'pach',
    LBM_LOGIN_REQUESTED,
    LBM_NOTIFY_SESSION_EVENT
};

class BSafeTextControl : public BTextControl
{
public:
    BSafeTextControl(const char* name, const char* label,
        const char* initialText, BMessage* message,
        uint32 flags = B_WILL_DRAW | B_NAVIGABLE);

    void MessageReceived(BMessage* message) override;
    void ReplaceChars(char c, size_t length);
};

class mLoginBox : public BView, public ThreadedClass
{
public:
                    mLoginBox(BRect frame, const LWSettings* settings);
    virtual         ~mLoginBox();
    virtual void    AttachedToWindow();
    virtual void    MessageReceived(BMessage* message);
    virtual void    Draw(BRect updateRect);
    void            RequestLogin();
private:
    bool            IsAbleToLogin(bool pwdlessmode);

    static int32    CallUpdateUIForm(void* data);
    static int32    CallUpdateUIErrorMsg(void* data);
    static int32    CallUpdateUIExpiredMsg(void* data);
    static int32    CallUpdateUIAccExpiredMsg(void* data);
    static int32    CallUpdateUINotAllowedMsg(void* data);
    static int32    CallUpdateUIPwdlessOffMsg(void* data);
    static int32    CallUpdateUILockdown(void* data);
    static int32    CallInactivityTimerWatcher(void* data);
    static void     OnExitResetForm(void* data);
    void            UpdateUIForm();
    void            LockUIForm();
    void            UnlockUIForm();
    void            UpdateUIErrorMsg(BString str);
    void            InactivityTimerWatcher();

private:
    BStringView     *errorView;
    BTextControl    *tcUserName;
    BSafeTextControl *tcPassword;
    BButton         *btLogin;

    thread_id       thUpdateUIForm,
                    thUpdateUILockdown,
                    thInactivityTimer,
                    thUpdateUIErrorMsg;

    bool            isPwdLessOn;
    bool            isInactivityTimerOn;
    int32           loginAttempts;
    int32           errorThreshold;
    int32           snoozeMultiplier;
    const uint32    inactivityTime;
};

#endif // _H
