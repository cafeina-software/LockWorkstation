/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <Catalog.h>
#include <string>
#include "mUserInfo.h"
#include "../common/AuthenticationUtils.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "mUserInfo dialog"

mUserInfo::mUserInfo(BRect frame, BWindow* parent, const char* name)
: BWindow(frame, B_TRANSLATE("User info: <invalid user>"),
    B_FLOATING_WINDOW, B_ASYNCHRONOUS_CONTROLS),
    fParentWindow(parent)
{
    float maxwidth, maxheight;
    GetSizeLimits(NULL, &maxwidth, NULL, &maxheight);
    SetSizeLimits(500.0f, maxwidth, 400.0f, maxheight);

    fTcUserName = new BTextControl(B_TRANSLATE("Username"), "", NULL);
    fTcUserName->TextView()->MakeEditable(false);
    fTcUserName->TextView()->MakeSelectable(false);
    fTcUserId = new BTextControl(B_TRANSLATE("User ID"), "", NULL);
    fTcUserId->TextView()->MakeEditable(false);
    fTcUserId->TextView()->MakeSelectable(false);
    fTcGroupId = new BTextControl(B_TRANSLATE("Group ID"), "", NULL);
    fTcGroupId->TextView()->MakeEditable(false);
    fTcGroupId->TextView()->MakeSelectable(false);
    fTcGecos = new BTextControl(B_TRANSLATE("User information"), "", NULL);
    fTcGecos->TextView()->MakeEditable(false);
    fTcGecos->TextView()->MakeSelectable(false);
    fTcHomeDir = new BTextControl(B_TRANSLATE("Home directory"), "", NULL);
    fTcHomeDir->TextView()->MakeEditable(false);
    fTcHomeDir->TextView()->MakeSelectable(false);
    fTcLoginShell = new BTextControl(B_TRANSLATE("Login shell"), "", NULL);
    fTcLoginShell->TextView()->MakeEditable(false);
    fTcLoginShell->TextView()->MakeSelectable(false);

    fTcOldPassword = new BTextControl(B_TRANSLATE("Current password"), "",
        NULL);
    fTcOldPassword->TextView()->HideTyping(true);
    fTcNewPassword = new BTextControl(B_TRANSLATE("New password"), "",
        new BMessage(UI_PWD_TEXT_MODIFIED));
    fTcNewPassword->TextView()->HideTyping(true);
    fTcNewPassword->SetModificationMessage(new BMessage(UI_PWD_TEXT_MODIFIED));
    fTcConfirmNewPassword = new BTextControl(B_TRANSLATE("Confirm new password"),
        "", new BMessage(UI_PWD_TEXT_MODIFIED));
    fTcConfirmNewPassword->TextView()->HideTyping(true);
    fTcConfirmNewPassword->SetModificationMessage(new BMessage(UI_PWD_TEXT_MODIFIED));
    fBtChangePwd = new BButton("bt_go", B_TRANSLATE("Change password"),
        new BMessage(UI_PWD_CHANGE_REQUESTED));

    containerView = new BView("v_pwdc", B_SUPPORTS_LAYOUT, NULL);
    containerView->SetViewUIColor(B_TOOL_TIP_BACKGROUND_COLOR);
    BLayoutBuilder::Group<>(containerView, B_VERTICAL, 0)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
            .SetInsets(0, 0, 0, B_USE_SMALL_INSETS)
            .AddTextControl(fTcOldPassword, 0, 0)
            .AddTextControl(fTcNewPassword, 0, 1)
            .AddTextControl(fTcConfirmNewPassword, 0, 2)
        .End()
        .AddGroup(B_HORIZONTAL)
            .AddGlue()
            .Add(fBtChangePwd)
        .End()
    .End();
    containerView->Hide();

    fBtShowPwdChangeView = new BButton("bt_change", B_TRANSLATE("Password"),
        new BMessage(UI_WINDOW_SHOW_PWDCHANGE));
    fBtShowPwdChangeView->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
    fBtCloseWindow = new BButton("bt_ok", B_TRANSLATE("Cerrar"),
        new BMessage(UI_WINDOW_CLOSE));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
            .SetInsets(0, 0, 0, B_USE_SMALL_INSETS)
            .AddTextControl(fTcUserName, 0, 0)
            .AddTextControl(fTcUserId, 0, 1)
            .AddTextControl(fTcGroupId, 0, 2)
            .AddTextControl(fTcGecos, 0, 3)
            .AddTextControl(fTcHomeDir, 0, 4)
            .AddTextControl(fTcLoginShell, 0, 5)
        .End()
        .AddGlue()
        .AddGroup(B_HORIZONTAL)
            .SetInsets(0, 0, 0, B_USE_SMALL_INSETS)
            .Add(containerView)
        .End()
        .AddGlue()
        .AddGroup(B_HORIZONTAL)
            .Add(fBtShowPwdChangeView)
            .AddGlue()
            .Add(fBtCloseWindow)
        .End()
    .End();

    _InitUIData(name);

    CenterIn(fParentWindow->Frame());
}

void mUserInfo::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case UICmds::UI_PWD_TEXT_MODIFIED:
        {
            bool result = _NewPasswordIsOK();
            fTcNewPassword->MarkAsInvalid(!result);
            fTcConfirmNewPassword->MarkAsInvalid(!result);
            fBtChangePwd->SetEnabled(result);
            break;
        }
        case UICmds::UI_PWD_CHANGE_REQUESTED:
        {
            if(!_NewPasswordIsOK()) {
                Alert(B_TRANSLATE("Warning"), B_TRANSLATE("New passwords do not match."),
                    B_WARNING_ALERT);
                break;
            }

            status_t status = try_change_pwd(fUserEntry->pw_name,
                fTcOldPassword->Text(), fTcConfirmNewPassword->Text());
            switch(status) {
                case B_PERMISSION_DENIED:
                    Alert(B_TRANSLATE("Error"), B_TRANSLATE("Authentication failed."),
                        B_STOP_ALERT);
                    break;
                case B_OK:
                    Alert(B_TRANSLATE("Good"),
                        B_TRANSLATE("Password was changed successfully."), B_INFO_ALERT);
                    Quit();
                    break;
                case B_BAD_VALUE:
                    Alert(B_TRANSLATE("Error"),
                        B_TRANSLATE("Bad data: username cannot be empty."),
                        B_STOP_ALERT);
                    break;
                default:
                    Alert(B_TRANSLATE("Error"),
                        B_TRANSLATE("Password could not be changed."), B_STOP_ALERT);
                    break;
            }
            break;
        }
        case UICmds::UI_WINDOW_SHOW_PWDCHANGE:
        {
            if(containerView->IsHidden()) {
                containerView->Show();
                fTcOldPassword->SetEnabled(true);
                fTcNewPassword->SetEnabled(true);
                fTcConfirmNewPassword->SetEnabled(true);
                fBtChangePwd->SetEnabled(_NewPasswordIsOK());
            }
            else {
                containerView->Hide();
                fTcOldPassword->SetEnabled(false);
                fTcNewPassword->SetEnabled(false);
                fTcConfirmNewPassword->SetEnabled(false);
                fBtChangePwd->SetEnabled(false);
            }
            ResizeToPreferred();
            break;
        }
        case UICmds::UI_WINDOW_CLOSE:
            Quit();
            break;
        default:
            BWindow::MessageReceived(message);
            break;
    }
}

void mUserInfo::_InitUIData(const char* username)
{
    fUserEntry = getpwnam(username);
    if(fUserEntry == NULL)
        return;
    fUserShadowEntry = getspnam(username);

    BString str;
    str.SetToFormat(B_TRANSLATE("User info: %s"), username);
    SetTitle(str.String());

    fTcUserName->SetText(fUserEntry->pw_name);
    fTcUserId->SetText(std::to_string(fUserEntry->pw_uid).c_str());
    fTcGroupId->SetText(std::to_string(fUserEntry->pw_gid).c_str());
    fTcGecos->SetText(fUserEntry->pw_gecos);
    fTcHomeDir->SetText(fUserEntry->pw_dir);
    fTcLoginShell->SetText(fUserEntry->pw_shell);
}

bool mUserInfo::_NewPasswordIsOK()
{
    return strcmp(fTcNewPassword->Text(), fTcConfirmNewPassword->Text()) == 0;
}
