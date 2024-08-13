/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <OS.h>
#include "mLoginBox.h"
#include "mConstant.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Login box"

static BString strLoginFailed = B_TRANSLATE("Login failed.");
static BString strLoginNotAllowed = B_TRANSLATE("Account has password login disabled.");
static BString strAccExpired = B_TRANSLATE("Account is expired.");
static BString strPassExpired = B_TRANSLATE("The password is expired.");
static BString strPwdlessOff = B_TRANSLATE("Empty password usage is disabled.");
static BString strNoError = "";

mLoginBox::mLoginBox(BRect frame, LWSettings* settings)
: BView(frame, "v_loginbox", B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE |
    B_NAVIGABLE_JUMP | B_INPUT_METHOD_AWARE | B_DRAW_ON_CHILDREN),
    loginAttempts(0),
    isPwdLessOn(settings->PasswordLessAuthEnabled()),
    errorThreshold(settings->AuthenticationAttemptsThreshold()),
    snoozeMultiplier(settings->AuthenticationCooldownAfterThreshold()),
    isInactivityTimerOn(settings->AuthenticationResetFormIfInactive()),
    inactivityTime(30) // 30 seconds
{
    SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    BFont font1(be_plain_font);
    font1.SetSize(be_plain_font->Size() * 2);
    BStringView* descriptionView = new BStringView("sv_desc",
        B_TRANSLATE("Session locked"));
    descriptionView->SetFont(&font1);

    BFont font2(be_plain_font);
    font2.SetSize(be_plain_font->Size() * 1.5);
    font2.SetFace(B_ITALIC_FACE);

    BStringView* instructionsView = new BStringView("sv_inst",
        B_TRANSLATE("Please write the user name and password to unlock"));
    instructionsView->SetFont(&font2);

    BFont font3;
    font3.SetSize(be_plain_font->Size() * (1.5));

    tcUserName = new BTextControl("tc_username", B_TRANSLATE("User name"), "",
        new BMessage('user'));
    const char blacklist [] = {"\0\a\b\t\n\v\f\r\e\x20"};
    for(int i = 0; i < sizeof(blacklist)/sizeof(blacklist[0]); i++)
        tcUserName->TextView()->DisallowChar(blacklist[i]);

    tcUserName->SetModificationMessage(new BMessage(LBM_USERNAME_CHANGED));
    tcUserName->SetFont(&font3);
    tcUserName->MakeFocus(true);
    tcPassword = new BTextControl("tc_password", B_TRANSLATE("Password"), "",
        new BMessage('pass'));
    tcPassword->SetModificationMessage(new BMessage(LBM_PASSWORD_CHANGED));
    tcPassword->SetFont(&font3);
    tcPassword->TextView()->HideTyping(true);
    btLogin = new BButton("bt_login", B_TRANSLATE("Login"),
        new BMessage(LBM_LOGIN_REQUESTED));
    btLogin->SetFont(&font3);
    btLogin->SetEnabled(IsAbleToLogin(isPwdLessOn));
    btLogin->MakeDefault(true);

    BFont errorFont(be_bold_font);
    errorFont.SetSize(be_bold_font->Size() * 1.5);

    errorView = new BStringView("sv_error", strNoError);
    errorView->SetFont(&errorFont);
    errorView->SetHighColor(ui_color(B_FAILURE_COLOR));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .SetInsets(B_USE_WINDOW_INSETS)
        .Add(descriptionView)
        .Add(instructionsView)
        .AddStrut(16.0f)
        .AddGrid()
            .AddTextControl(tcUserName, 0, 0)
            .AddTextControl(tcPassword, 0, 1)
        .End()
        .AddStrut(12.0f)
        .AddGroup(B_HORIZONTAL)
            .Add(errorView)
            .AddGlue()
            .Add(btLogin)
        .End()
    .End();

    ResizeToPreferred();
}

mLoginBox::~mLoginBox()
{
}

// #pragma mark -

void mLoginBox::AttachedToWindow()
{
    tcUserName->SetTarget(this);
    tcPassword->SetTarget(this);
    btLogin->SetTarget(this);
}

void mLoginBox::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case 'user':
        case loginBoxMsgs::LBM_USERNAME_CHANGED:
            ThreadedCall(thUpdateUIForm, CallUpdateUIForm,
                "Update error message", B_NORMAL_PRIORITY, this);
            if(isInactivityTimerOn) {
                kill_thread(thInactivityTimer); // nothing to be worth of saving
                ThreadedCall(thInactivityTimer, CallInactivityTimerWatcher,
                    "Inactivity watcher thread", B_LOW_PRIORITY, this);
            }
            break;
        case 'pass':
        case loginBoxMsgs::LBM_PASSWORD_CHANGED:
            ThreadedCall(thUpdateUIForm, CallUpdateUIForm,
                "Update error message", B_NORMAL_PRIORITY, this);
            if(isInactivityTimerOn) {
                kill_thread(thInactivityTimer); // nothing to be worth of saving
                ThreadedCall(thInactivityTimer, CallInactivityTimerWatcher,
                    "Inactivity watcher thread", B_LOW_PRIORITY, this);
            }
            break;
        case loginBoxMsgs::LBM_LOGIN_REQUESTED:
            if(!isPwdLessOn && tcPassword->TextLength() == 0) {
                ThreadedCall(thUpdateUIErrorMsg, CallUpdateUIPwdlessOffMsg,
                    "Update error message", B_NORMAL_PRIORITY, this);
            }
            else {
                if(IsAbleToLogin(isPwdLessOn))
                    RequestLogin();
            }
            break;
        case M_LOGIN_FAILED:
            fprintf(stderr, "Login failed.\n");

            int32 (*fncall)(void*);
            switch(message->GetInt32("errorCode", B_ERROR))
            {
                case B_NOT_ALLOWED:
                    fncall = CallUpdateUIAccExpiredMsg;
                    break;
                case B_TIMED_OUT:
                    fncall = CallUpdateUIExpiredMsg;
                    break;
                case B_PERMISSION_DENIED:
                    fncall = CallUpdateUINotAllowedMsg;
                    break;
                case B_ERROR:
                default:
                    fncall = CallUpdateUIErrorMsg;
                    break;
            }

            ThreadedCall(thUpdateUIErrorMsg, fncall,
                "Update error message", B_NORMAL_PRIORITY, this);

            // Inspired by uber-perfect-cell: use fail count feature to prevent
            //  brute-force attacks. After a number of failed attempts,
            //  it will cool down for a few seconds after which the user
            //  is able to try again.
            ++loginAttempts;
            if(errorThreshold > 0 && snoozeMultiplier > 0 &&
            loginAttempts >= 1 && loginAttempts % errorThreshold == 0)
                ThreadedCall(thUpdateUILockdown, CallUpdateUILockdown,
                    "Update UI after too many errors", B_NORMAL_PRIORITY, this);
            break;
        default:
            BView::MessageReceived(message);
            break;
    }
}

void mLoginBox::Draw(BRect updateRect)
{
    BView::Draw(updateRect);
    Invalidate();
}

void mLoginBox::RequestLogin()
{
    BMessage message(BUTTON_LOGIN);
    message.AddString("username", tcUserName->Text());
    message.AddString("password", tcPassword->Text());
    Window()->PostMessage(&message, Window(), this);
}

// #pragma mark -

bool mLoginBox::IsAbleToLogin(bool pwdlessmode)
{
    return tcUserName->TextLength() > 0/* &&
           (pwdlessmode || tcPassword->TextLength() > 0)*/;
}

int32 mLoginBox::CallUpdateUIForm(void* data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->UpdateUIForm();
    return B_OK;
}

int32 mLoginBox::CallUpdateUIErrorMsg(void* data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->UpdateUIErrorMsg(strLoginFailed);
    return B_OK;
}

int32 mLoginBox::CallUpdateUIExpiredMsg(void *data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->UpdateUIErrorMsg(strPassExpired);
    return B_OK;
}

int32 mLoginBox::CallUpdateUIAccExpiredMsg(void *data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->UpdateUIErrorMsg(strAccExpired);
    return B_OK;
}

int32 mLoginBox::CallUpdateUINotAllowedMsg(void *data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->UpdateUIErrorMsg(strLoginNotAllowed);
    return B_OK;
}

int32 mLoginBox::CallUpdateUIPwdlessOffMsg(void* data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->UpdateUIErrorMsg(strPwdlessOff);
    return B_OK;
}

int32 mLoginBox::CallUpdateUILockdown(void* data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->LockUIForm();
    snooze(box->snoozeMultiplier * 1000000);
    box->UnlockUIForm();
    return B_OK;
}

int32 mLoginBox::CallInactivityTimerWatcher(void* data)
{
    mLoginBox* box = (mLoginBox*)data;
    if(box == nullptr)
        return B_ERROR;

    box->InactivityTimerWatcher();
    return B_OK;
}

void mLoginBox::UpdateUIForm()
{
    LockLooper();

    tcUserName->MarkAsInvalid(!(tcUserName->TextLength() > 0));
    tcPassword->MarkAsInvalid(!(isPwdLessOn || tcPassword->TextLength() > 0));
    btLogin->SetEnabled(IsAbleToLogin(isPwdLessOn));

    UnlockLooper();
}

void mLoginBox::LockUIForm()
{
    LockLooper();

    tcUserName->SetEnabled(false);
    tcPassword->SetEnabled(false);
    btLogin->SetEnabled(false);
    Invalidate();
    Window()->UpdateIfNeeded();

    UnlockLooper();
}

void mLoginBox::UnlockUIForm()
{
    LockLooper();

    tcUserName->SetEnabled(true);
    tcPassword->SetEnabled(true);
    OnExitResetForm(this);
    UpdateUIErrorMsg(strNoError);
    btLogin->SetEnabled(IsAbleToLogin(isPwdLessOn));
    Invalidate();
    Window()->UpdateIfNeeded();

    UnlockLooper();
}

void mLoginBox::UpdateUIErrorMsg(BString whatstr)
{
    LockLooper();
    errorView->SetText(whatstr);
    UnlockLooper();
}

void mLoginBox::InactivityTimerWatcher()
{
   if(isInactivityTimerOn) {
        snooze(inactivityTime * 1000000);
        on_exit_thread(&mLoginBox::OnExitResetForm, this);
    }
}

void mLoginBox::OnExitResetForm(void* data)
{
    mLoginBox* box = (mLoginBox*)data;

    box->LockLooper();
    box->tcUserName->SetText("");
    box->tcPassword->SetText("");
    box->UnlockLooper();
}
