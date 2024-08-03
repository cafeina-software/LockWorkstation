/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _mUserInfo_h_
#define _mUserInfo_h_

#include <InterfaceKit.h>
#include <SupportKit.h>
#include <pwd.h>
#include <shadow.h>

enum UICmds {
    UI_WINDOW_CLOSE          = 'clse',
    UI_WINDOW_SHOW_PWDCHANGE = 'pwdc',
    UI_PWD_TEXT_MODIFIED     = 'modf',
    UI_PWD_CHANGE_REQUESTED  = 'chng'
};

inline int32 Alert(const char* title, const char* text, alert_type type = B_INFO_ALERT) {
    BAlert* alert = new BAlert(title, text, "OK");
    alert->SetType(type);
    return alert->Go();
}

class mUserInfo : public BWindow
{
public:
                    mUserInfo(BRect frame, BWindow* parent, const char* name);
    virtual void    MessageReceived(BMessage* message);
private:
    void            _InitUIData(const char* username);
    status_t        _RequestChange();
    bool            _NewPasswordIsOK();
private:
    struct passwd  *fUserEntry;
    struct spwd    *fUserShadowEntry;
    BWindow        *fParentWindow;
    BView          *containerView;
    BButton        *fBtChangePwd,
                   *fBtShowPwdChangeView,
                   *fBtCloseWindow;
    BTextControl   *fTcUserName,
                   *fTcUserId,
                   *fTcGecos,
                   *fTcGroupId,
                   *fTcHomeDir,
                   *fTcLoginShell,
                   *fTcOldPassword,
                   *fTcNewPassword,
                   *fTcConfirmNewPassword;
};

#endif /* _mUserInfo_h_ */
