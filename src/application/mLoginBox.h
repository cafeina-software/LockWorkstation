/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _M_LOGIN_BOX_H
#define _M_LOGIN_BOX_H

#include <InterfaceKit.h>
#include <SupportDefs.h>
#include "../common/ThreadedClass.h"

enum loginBoxMsgs {
    LBM_USERNAME_CHANGED = 'usch',
    LBM_PASSWORD_CHANGED = 'pach',
    LBM_LOGIN_REQUESTED
};

class mLoginBox : public BView, public ThreadedClass
{
public:
                    mLoginBox(BRect frame, bool pwdless = true);
    virtual         ~mLoginBox();
    virtual void    AttachedToWindow();
    virtual void    MessageReceived(BMessage* message);
    virtual void    Draw(BRect updateRect);
    void            RequestLogin();
private:
    bool            IsAbleToLogin(bool pwdlessmode);

    static int      CallUpdateUIForm(void* data);
    static int      CallUpdateUIErrorMsg(void* data);
    static int      CallUpdateUIExpiredMsg(void* data);
    static int      CallUpdateUIAccExpiredMsg(void* data);
    static int      CallUpdateUINotAllowedMsg(void* data);
    static int      CallUpdateUIPwdlessOffMsg(void* data);
    void            UpdateUIForm();
    void            UpdateUIErrorMsg(BString str);
private:
    BStringView     *errorView;
    BTextControl    *tcUserName,
                    *tcPassword;
    BButton         *btLogin;

    thread_id       thUpdateUIForm,
                    thUpdateUIErrorMsg;

    bool            isPwdLessOn;
};

#endif // _H
