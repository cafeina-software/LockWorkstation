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
    LBM_LOGIN_REQUESTED
};

class mLoginBox : public BView, public ThreadedClass
{
public:
                    mLoginBox(BRect frame, LWSettings* settings);
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
    void            UpdateUIForm();
    void            LockUIForm();
    void            UnlockUIForm();
    void            UpdateUIErrorMsg(BString str);
private:
    BStringView     *errorView;
    BTextControl    *tcUserName,
                    *tcPassword;
    BButton         *btLogin;

    thread_id       thUpdateUIForm,
                    thUpdateUILockdown,
                    thUpdateUIErrorMsg;

    bool            isPwdLessOn;
    int32           loginAttempts;
    int32           snoozeMultiplier;
    int32           errorThreshold;
};

#endif // _H
