/**********************************************************************************/
/*****************        Lock Workstation By Marius Stene        *****************/
/*****************             m_stene@yahoo.com, 2002            *****************/
/*****************        My contribution to this great OS        *****************/
/**********************************************************************************/
/*****************  You are welcome to use any part of this code  *****************/
/*****************      in whole, or in parts freely without      *****************/
/*****************                  contacting me                 *****************/
/**********************************************************************************/
//Global
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <private/interface/Spinner.h>
#include <cstdio>
#include <string>
//Local
#include "mWindow.h"
#include "../common/LockWorkstationConfig.h"
#include "../common/AuthenticationUtils.h"

const BRect mWindowRect 					(64, 64, 504, 424);

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "mWindow"

//Constructa
mWindow::mWindow(const char *mWindowTitle)
: BWindow(BRect(mWindowRect), mWindowTitle, B_TITLED_WINDOW,
    B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_RESIZABLE),
    fInfoWnd(nullptr)
{
    InitUIData(); // Load data from config file

    amthdCardView = CreateCardView_AccountMethod();
    userCardView  = CreateCardView_User();
	bgCardView    = CreateCardView_Background();
	clockCardView = CreateCardView_Clock();
    logCardView   = CreateCardView_Logging();
	extraCardView = CreateCardView_Options();

    //I te yo evythin
    mButtonApplyEverything = new BButton("mFrameButtonApplyEverything",
        B_TRANSLATE("Apply"), new BMessage(APPLY_EVERYTHING), B_WILL_DRAW | B_NAVIGABLE);
    mEraserButtonOfDoom = new BButton("EraserButtonOfDoom",
        B_TRANSLATE("Default"), new BMessage(ERASER_FROM_DOOM));

    mApplyView = new BView(BRect(0, 322, 500, 360), "ApplyView", B_FOLLOW_ALL_SIDES,
        B_WILL_DRAW | B_NAVIGABLE);
    mApplyView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    mApplyView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(mApplyView, B_HORIZONTAL)
        .AddGlue()
        .Add(mEraserButtonOfDoom)
        .Add(mButtonApplyEverything)
    .End();

    // Main view
    fPanelList = new BListView(B_SINGLE_SELECTION_LIST);
    fPanelList->SetSelectionMessage(new BMessage(M_ITEM_SELECTED));
    fPanelList->AddItem(new BStringItem(B_TRANSLATE("Authentication")));
    fPanelList->AddItem(new BStringItem(B_TRANSLATE("Accounts")));
    fPanelList->AddItem(new BStringItem(B_TRANSLATE("Background")));
    fPanelList->AddItem(new BStringItem(B_TRANSLATE("Clock")));
    fPanelList->AddItem(new BStringItem(B_TRANSLATE("Event log")));
    fPanelList->AddItem(new BStringItem(B_TRANSLATE("Options")));
    BScrollView *listScrollView = new BScrollView("sc_cont", fPanelList,
        0, false, true, B_FANCY_BORDER);
    fPanelList->SetExplicitMinSize(
        BSize(fPanelList->StringWidth(B_TRANSLATE("Authentication")) * 1.1,
        B_SIZE_UNSET));

    fCardView = new BCardView();
    fCardView->AddChild(amthdCardView);
    fCardView->AddChild(userCardView);
    fCardView->AddChild(bgCardView);
    fCardView->AddChild(clockCardView);
    fCardView->AddChild(logCardView);
    fCardView->AddChild(extraCardView);

    BLayoutBuilder::Group<>(this, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGroup(B_HORIZONTAL)
            .Add(listScrollView)
            .Add(fCardView)
        .End()
        .Add(mApplyView)
    .End();

    InitUIControls(); // Set controls' values to the data values
	DisplayCard(0);

    BMessenger msgr(this);
    be_roster->StartWatching(msgr, B_SOME_APP_ACTIVATED);

    mFilePanelFolderBrowse = new BFilePanel(B_OPEN_PANEL, &msgr, &mEntryRef,
        -1, false, NULL, NULL, true, true);

    AddShortcut('A', B_COMMAND_KEY, new BMessage(B_ABOUT_REQUESTED));
    AddShortcut('S', B_COMMAND_KEY, new BMessage(APPLY_EVERYTHING));
    AddShortcut('D', B_COMMAND_KEY, new BMessage(ERASER_FROM_DOOM));

    //Enable and disable buttons
    ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
        "Enable and disable buttons", B_LOW_PRIORITY, this);

    CenterOnScreen();
}

mWindow::~mWindow()
{
    delete mFilePanelFolderBrowse;
    delete settings;
}

void mWindow::MessageReceived(BMessage* message)
{
    switch(message->what)
	{
        case M_ITEM_SELECTED:
        {
            int32 index = message->GetInt32("index", 0);
            if (index >= 0 && index < fCardView->CountChildren())
				fCardView->CardLayout()->SetVisibleItem(index);
            break;
        }
        case M_AUTHMTHD_SYSTEM:
        {
            settings->SetAuthenticationMethod(AUTH_SYSTEM_ACCOUNT);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_AUTHMTHD_APPACC:
        {
            settings->SetAuthenticationMethod(AUTH_APP_ACCOUNT);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_AUTHOPTS_THRSHD:
        {
            settings->SetAuthenticationAttemptsThreshold(mSliderAttemptsThrshld->Value());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_AUTHOPTS_ERRSNZ:
        {
            settings->SetAuthenticationCooldownAfterThreshold(mSliderErrorWaitTime->Value());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_AUTHOPTS_RESETFORM:
        {
            bool result = mCheckBoxResetLoginForm->Value() == B_CONTROL_ON;
            settings->SetAuthenticationResetFormIfInactive(result);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_APPUSER_IVK:
        {
            int32 index = mListOfUsers->CurrentSelection();
            if(index >= 0 && index < mListOfUsers->CountItems()) {
                const char* name = ((BStringItem*)mListOfUsers->ItemAt(index))->Text();
                if(system_has_user(name)) {
                    fInfoWnd = new mUserInfo(BRect(0, 0, 400, 300), this, name);
                    fInfoWnd->Show();
                }
                else {
                    ((new BAlert("Error", "User does not exist.", "OK")))->Go();
                }
            }
            break;
        }
        /*****************************************************************************/
        /******************************** DIE!!! *************************************/
        /*****************************************************************************/
        case ERASER_FROM_DOOM:
        {
            settings->Reset();
            settings->SaveSettings();
            InitUIData();
            InitUIControls();

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        /********************************************************************/
        /************************** LIVE! ***********************************/
        /********************************************************************/
        case APPLY_EVERYTHING:
        {
            settings->Commit();
            settings->SaveSettings();

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case BOOL_CLOCK:
        {
            settings->SetClockEnabled(mCheckBoxBoolClock->Value() == B_CONTROL_ON);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BOOL_SESSION:
        {
            settings->SetSessionBarEnabled(mCheckBoxSessionBar->Value() == B_CONTROL_ON);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BOOL_INFO:
        {
            settings->SetSystemInfoPanelEnabled(mCheckBoxSysInfo->Value() == B_CONTROL_ON);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BOOL_KILLER:
        {
            settings->SetKillerShortcutEnabled(mCheckBoxKillerShortcut->Value() == B_CONTROL_ON);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_BOOL:
        {
            settings->SetEventLogEnabled(mCheckBoxEventLog->Value() == B_CONTROL_ON);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BOOL_PWDLESS:
        {
            settings->SetPasswordLessAuthEnabled(mCheckBoxAllowPwdlessLogin->Value() == B_CONTROL_ON);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGCOLOR_CHANGED:
        {
            settings->SetBackgroundColor(mCCBgColor->ValueAsColor());
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_CLOCK_COLORCHANGED:
        {
            settings->SetClockColor(mCCClockColor->ValueAsColor());
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case DEFAULT_COLORS:
        {
            BMessage* temp = new BMessage;
            LWSettings::DefaultSettings(temp);

            settings->SetBackgroundColor(temp->GetColor(mNameConfigBgColor, {}));
            mCCBgColor->SetValue(settings->BackgroundColor());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);

            delete temp;
            break;
        }
        case DEFAULT_CLOCK_COLORS:
        {
            BMessage* temp = new BMessage;
            LWSettings::DefaultSettings(temp);

            settings->SetClockColor(temp->GetColor(mNameConfigClockColor, {}));
            mCCClockColor->SetValue(settings->ClockColor());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);

            delete temp;
            break;
        }
        case CLOCKPLACEMENT_X:
        {
            BScreen screen(B_MAIN_SCREEN_ID);
            float x = atof(mTextControlClockPlaceX->Text());

            if(x >= 0 && x < screen.Frame().Width())
                settings->SetClockLocation(BPoint(x, settings->ClockLocation().y));
            else
                settings->SetClockLocation(BPoint(0, settings->ClockLocation().y));

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case CLOCKPLACEMENT_Y:
        {
            BScreen screen(B_MAIN_SCREEN_ID);
            float y = atof(mTextControlClockPlaceY->Text());

            if(y >= 0 && y < screen.Frame().Height())
                settings->SetClockLocation(BPoint(settings->ClockLocation().x, y));
            else
                settings->SetClockLocation(BPoint(settings->ClockLocation().x, 0));

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case DEFAULT_PLACE:
        {
            BMessage* temp = new BMessage;
            LWSettings::DefaultSettings(temp);

            settings->SetClockLocation(temp->GetPoint(mNameConfigClockPlace, BPoint()));
            mTextControlClockPlaceX->SetText(std::to_string(settings->ClockLocation().x).c_str());
            mTextControlClockPlaceY->SetText(std::to_string(settings->ClockLocation().y).c_str());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);

            delete temp;
            break;
        }
        case SIZE_SLIDER_CHANGED:
        {
            settings->SetClockSize(mSliderFontSize->Value());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_SNOOZE_CHANGED:
        {
            settings->SetBackgroundImageSnooze(mSliderBgSnooze->Value());
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_ADJUST_KEEP:
        {
            settings->SetBackgroundImageAdjustment(BGI_ADJ_KEEP_AND_CENTER);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_ADJUST_X:
        {
            settings->SetBackgroundImageAdjustment(BGI_ADJ_SCALE_X);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_ADJUST_Y:
        {
            settings->SetBackgroundImageAdjustment(BGI_ADJ_SCALE_Y);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_ADJUST_X_Y:
        {
            settings->SetBackgroundImageAdjustment(BGI_ADJ_SCALE_X_Y);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_ADJUST_STRETCH:
        {
            settings->SetBackgroundImageAdjustment(BGI_ADJ_STRETCH_TO_SCREEN);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case CHANGE_LOGIN:
        {
            settings->SetDefaultUser(mAddUserName->Text());
        	settings->SetDefaultUserPassword(mAddPassWord->Text());
        	//Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_BROWSE_SINGLE:
        {
            ImageFilter* filter = new ImageFilter;
            CallBgImgFilePanel(STATIC_CHANGED, B_FILE_NODE, filter);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_BROWSE_FOLDER:
        {
            CallBgImgFilePanel(FOLDER_CHANGED, B_DIRECTORY_NODE, NULL);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_BGIMG_BROWSE_LIST:
        {
            ListFilter* filter = new ListFilter;
            CallBgImgFilePanel(LIST_CHANGED, B_FILE_NODE, filter);
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case STATIC_CHANGED:
        {
            if(message->FindRef("refs", &mEntryRef) == B_OK) {
                BEntry entry(&mEntryRef);
                BPath path;
                entry.GetPath(&path);
                settings->SetBackgroundImageStatic(path.Path());
                mTextControlmPathToImage->SetText(settings->BackgroundImageStaticPath());
            }
            break;
        }
        case FOLDER_CHANGED:
        {
            if(message->FindRef("refs", &mEntryRef) == B_OK) {
                BEntry entry(&mEntryRef);
                BPath path;
                entry.GetPath(&path);
                settings->SetBackgroundImageFolderPath(path.Path());
                mTextControlmPathToImageFolder->SetText(settings->BackgroundImageFolderPath());
            }
            break;
        }
        case LIST_CHANGED:
        {
            if(message->FindRef("refs", &mEntryRef) == B_OK) {
                BEntry entry(&mEntryRef);
                BPath path;
                entry.GetPath(&path);
                settings->SetBackgroundImageListPath(path.Path());
                mTextControlmPathToImageList->SetText(settings->BackgroundImageListPath());
            }
            break;
        }
        case CHECK_BUTTONS:
            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        case CHECK_USERBUTTON:
            ThreadedCall(EnDUserButtonThread, EnDUserButtonThread_static,
                "Enable and disable user button", B_LOW_PRIORITY, this);
            break;
        case M_BGMODE_NONE:
            settings->SetBackgroundMode(BGM_NONE);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        case M_BGMODE_SINGLE:
            settings->SetBackgroundMode(BGM_STATIC);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        case M_BGMODE_FOLDER:
            settings->SetBackgroundMode(BGM_FOLDER);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        case M_BGMODE_LIST:
            settings->SetBackgroundMode(BGM_LISTFILE);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        case M_EVTLOG_CLEAR:
        {
            BString response("");
            BPath path;
            find_directory(B_SYSTEM_LOG_DIRECTORY, &path);
            path.Append("LockWorkstation.log");
            BEntry logfile(path.Path());

            if(logfile.Exists()) {
                status_t status = logfile.Remove();
                if(status == B_OK)
                    response.SetTo("Logs cleared successfully.");
                else if(status == B_NO_INIT)
                    response.SetTo("The log file does not exist.");
                else
                    response.SetTo("There was an error while trying to delete log file.");
            }
            else
                response.SetTo("The log file does not exist.");

            mStaticLogClrResponse->SetText(response.String());
            break;
        }
        case M_EVTLOG_LEVEL_1:
        {
            settings->SetEventLogLevel(1);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_LEVEL_2:
        {
            settings->SetEventLogLevel(2);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_LEVEL_3:
        {
            settings->SetEventLogLevel(3);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_POLICY_0:
        {
            settings->SetEventLogRetentionPolicy(EVP_CONTINUE);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_POLICY_1:
        {
            settings->SetEventLogRetentionPolicy(EVP_WIPE_AFTER_SIZE);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_POLICY_2:
        {
            settings->SetEventLogRetentionPolicy(EVP_WIPE_AFTER_AGE);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_POLICY_SIZE:
        {
            settings->SetEventLogMaxSize(mSpinnerLogMaxSize->Value());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_EVTLOG_POLICY_AGE:
        {
            settings->SetEventLogMaxAge(mSpinnerLogMaxAge->Value());

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case M_UTIL_ADDBOOT:
        {
            EnableAutoStart(true);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);

            break;
        }
        case M_UTIL_REMBOOT:
        {
            EnableAutoStart(false);

            //Enable and disable buttons
            ThreadedCall(EnDButtonsThread, EnDButtonsThread_static,
                "Enable and disable buttons", B_LOW_PRIORITY, this);
            break;
        }
        case B_ABOUT_REQUESTED:
            be_app->PostMessage(B_ABOUT_REQUESTED);
            break;
        default:
            BWindow::MessageReceived(message);
            break;
	}
}

bool mWindow::QuitRequested()
{
    be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

// #pragma mark -

int32
mWindow::EnDButtonsThread_static(void *data)
{
	mWindow *enabuttons = (mWindow *)data;
	enabuttons->EnDButtons_Thread();
	return 0;
}

void mWindow::EnDButtons_Thread()
{
    snooze(10000);

    BMessage* defaults = new BMessage;
    LWSettings::DefaultSettings(defaults);

    bgCardView->LockLooper();
    mButtonDefaultColors->SetEnabled(!UI_IsBgColorDefault(defaults));
    bgCardView->UnlockLooper();

    clockCardView->LockLooper();
    UI_ClockControlsEnable(defaults, mCheckBoxBoolClock->Value() == B_CONTROL_ON);
    clockCardView->UnlockLooper();

    logCardView->LockLooper();
    UI_LogControlsEnable(mCheckBoxEventLog->Value() == B_CONTROL_ON);
    logCardView->UnlockLooper();

    extraCardView->LockLooper();
    mButtonAddToBoot->SetEnabled(HasAutoStartInstalled() == B_ENTRY_NOT_FOUND);
    mButtonRemFromBoot->SetEnabled(HasAutoStartInstalled() != B_ENTRY_NOT_FOUND);
    extraCardView->UnlockLooper();

    mApplyView->LockLooper();
    mEraserButtonOfDoom->SetEnabled(!UI_IsDefault(defaults));
    mButtonApplyEverything->SetEnabled(settings->HasPendingData());
    mApplyView->UnlockLooper();

    delete defaults;
}

/*******Enable and Disable User Button**************/
int32
mWindow::EnDUserButtonThread_static(void *data)
{
	mWindow *enauserbutton = (mWindow *)data;
	enauserbutton->EnDUserButton_Thread();
	return 0;
}

/*************************************************/
void mWindow::EnDUserButton_Thread()
{
    bool validUser = strcmp(mAddUserName->Text(), "") != 0;
    bool verifiedPwd = strcmp(mAddPassWord->Text(), mAddPassWordRetype->Text()) == 0 &&
                       strcmp(mAddPassWord->Text(), "") != 0;

    userCardView->LockLooper();
    mButtonChangeLogin->SetEnabled(validUser && verifiedPwd);
    userCardView->UnlockLooper();
}

int32
mWindow::UpdateUIThread_static(void *data)
{
	mWindow *updatestrings = (mWindow *)data;
	updatestrings->UpdateUI_Thread();
	return 0;
}

void mWindow::UpdateUI_Thread()
{
    InitUIControls();
}

void mWindow::InitUIControls()
{
    LockLooper();

    switch(settings->AuthenticationMethod()) {
        case 0:
            mRadioBtAuthSysaccount->SetValue(B_CONTROL_ON);
            mRadioBtAuthAppaccount->SetValue(B_CONTROL_OFF);
            break;
        case 2:
        default:
            mRadioBtAuthSysaccount->SetValue(B_CONTROL_OFF);
            mRadioBtAuthAppaccount->SetValue(B_CONTROL_ON);
            break;
    }
    mCheckBoxAllowPwdlessLogin->SetValue(settings->PasswordLessAuthEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);
    mSliderAttemptsThrshld->SetValue(settings->AuthenticationAttemptsThreshold());
    mSliderErrorWaitTime->SetValue(settings->AuthenticationCooldownAfterThreshold());
    mCheckBoxResetLoginForm->SetValue(settings->AuthenticationResetFormIfInactive() ?
        B_CONTROL_ON : B_CONTROL_OFF);

    mAddUserName->SetText(settings->DefaultUser());
    mAddPassWord->SetText(settings->DefaultUserPassword());
    mAddPassWordRetype->SetText(settings->DefaultUserPassword());
    BStringList users = get_system_users();
    mListOfUsers->MakeEmpty();
    for(int i = 0; i < users.CountStrings(); i++)
        mListOfUsers->AddItem(new BStringItem(users.StringAt(i).String()));

    mCCBgColor->SetValue(settings->BackgroundColor());
    switch(settings->BackgroundMode()) {
        case BGM_STATIC:
            mMfBgImageOption->Menu()->ItemAt(2)->SetMarked(true);
            break;
        case BGM_FOLDER:
            mMfBgImageOption->Menu()->ItemAt(3)->SetMarked(true);
            break;
        case BGM_LISTFILE:
            mMfBgImageOption->Menu()->ItemAt(4)->SetMarked(true);
            break;
        case BGM_NONE:
        default:
            mMfBgImageOption->Menu()->ItemAt(0)->SetMarked(true);
            break;
    }
    mTextControlmPathToImage->SetText(settings->BackgroundImageStaticPath());
    mTextControlmPathToImageFolder->SetText(settings->BackgroundImageFolderPath());
    mTextControlmPathToImageList->SetText(settings->BackgroundImageListPath());
    mSliderBgSnooze->SetValue(settings->BackgroundImageSnooze());
    switch(settings->BackgroundImageAdjustment()) {
        case BGI_ADJ_KEEP_AND_CENTER:
            mMfBgImageAdjustment->Menu()->ItemAt(BGI_ADJ_KEEP_AND_CENTER)->SetMarked(true);
            break;
        case BGI_ADJ_STRETCH_TO_SCREEN:
            mMfBgImageAdjustment->Menu()->ItemAt(BGI_ADJ_STRETCH_TO_SCREEN)->SetMarked(true);
            break;
        case BGI_ADJ_SCALE_X:
            mMfBgImageAdjustment->Menu()->ItemAt(BGI_ADJ_SCALE_X)->SetMarked(true);
            break;
        case BGI_ADJ_SCALE_Y:
            mMfBgImageAdjustment->Menu()->ItemAt(BGI_ADJ_SCALE_Y)->SetMarked(true);
            break;
        case BGI_ADJ_SCALE_X_Y:
        default:
            mMfBgImageAdjustment->Menu()->ItemAt(BGI_ADJ_SCALE_X_Y)->SetMarked(true);
            break;
    }

    mCheckBoxBoolClock->SetValue(settings->ClockIsEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);
    mSliderFontSize->SetValue(settings->ClockSize());
    mCCClockColor->SetValue(settings->ClockColor());
    mTextControlClockPlaceX->SetText(std::to_string(settings->ClockLocation().x).c_str());
    mTextControlClockPlaceY->SetText(std::to_string(settings->ClockLocation().y).c_str());
    BMessage msg;
    LWSettings::DefaultSettings(&msg);
    UI_ClockControlsEnable(&msg, settings->ClockIsEnabled());

    mCheckBoxEventLog->SetValue(settings->EventLogIsEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);
    switch(settings->EventLogLevel()) { // EVT_CRITICAL is reserved for the future
        case EVT_ERROR:
            mMfLogLevel->Menu()->ItemAt(0)->SetMarked(true);
            break;
        case EVT_WARNING:
            mMfLogLevel->Menu()->ItemAt(1)->SetMarked(true);
            break;
        case EVT_INFO:
        default:
            mMfLogLevel->Menu()->ItemAt(2)->SetMarked(true);
            break;
    }
    switch(settings->EventLogRetentionPolicy()) {
        case EVP_WIPE_AFTER_SIZE:
            mMfLogRetentionPolicy->Menu()->ItemAt(EVP_WIPE_AFTER_SIZE)->SetMarked(true);
            break;
        case EVP_WIPE_AFTER_AGE:
            mMfLogRetentionPolicy->Menu()->ItemAt(EVP_WIPE_AFTER_AGE)->SetMarked(true);
            break;
        case EVP_CONTINUE:
        default:
            mMfLogRetentionPolicy->Menu()->ItemAt(EVP_CONTINUE)->SetMarked(true);
            break;
    }
    mSpinnerLogMaxSize->SetValue(settings->EventLogMaxSize());
    mSpinnerLogMaxAge->SetValue(settings->EventLogMaxAge());
    UI_LogControlsEnable(settings->EventLogIsEnabled());

    mCheckBoxSessionBar->SetValue(settings->SessionBarIsEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);
    mCheckBoxSysInfo->SetValue(settings->SystemInfoPanelIsEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);
    mCheckBoxKillerShortcut->SetValue(settings->KillerShortcutIsEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);

    mButtonAddToBoot->SetEnabled(HasAutoStartInstalled() == B_ENTRY_NOT_FOUND);
    mButtonRemFromBoot->SetEnabled(HasAutoStartInstalled() != B_ENTRY_NOT_FOUND);

    UnlockLooper();
}

void mWindow::InitUIData()
{
    BPath path;
    find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    path.Append(mPathToConfigFile);

    settings = new LWSettings(path.Path());
}

status_t mWindow::DisplayCard(int32 index)
{
    if(!fPanelList)
        return B_ERROR;

    if(index < 0 || index >= fPanelList->CountItems())
        return B_BAD_INDEX;

    fPanelList->Select(index);
    fCardView->CardLayout()->SetVisibleItem(index);
    return B_OK;
}

// #pragma mark - UI Building

BView* mWindow::CreateCardView_AccountMethod()
{
    mRadioBtAuthSysaccount = new BRadioButton("rb_sysacc",
        B_TRANSLATE("Authenticate using a system account"),
        new BMessage(M_AUTHMTHD_SYSTEM));
    mRadioBtAuthSysaccount->SetFont(be_bold_font);
    BStringView* sysaccountDesc = new BStringView(NULL,
        B_TRANSLATE_COMMENT(
            "This authentication method requires an existing user account\n"
            "in the system to work. The user account must not be a \n"
            "service account.", "Please place the new line characters"
            "accordingly to accomodate the string, in order to not stretch "
            "the window too much")
    );

    mRadioBtAuthAppaccount = new BRadioButton("rb_appacc",
        B_TRANSLATE("Authenticate using an application based account (default)"),
        new BMessage(M_AUTHMTHD_APPACC));
    mRadioBtAuthAppaccount->SetFont(be_bold_font);
    BStringView* appaccountDesc = new BStringView(NULL,
        B_TRANSLATE_COMMENT(
            "This authentication method will make use of the username-password\n"
            "pair saved in this application's settings file. To manage the \n"
            "application account, please make use of this application's view\n"
            "\"Accounts\". Please consider that deleting or restoring \n"
            "the default values may delete this application-based account.",
            "Please place the new line characters accordingly to accomodate the"
            " string, in order to not stretch the window too much")
    );

    BView* authMethodView = new BView("v_acc_mthd", B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(authMethodView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(mRadioBtAuthSysaccount)
        .Add(sysaccountDesc)
        .Add(mRadioBtAuthAppaccount)
        .Add(appaccountDesc)
        .AddGlue()
    .End();

    BBox* mBoxAuthMethod = new BBox("box_authmthd",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, authMethodView);
    mBoxAuthMethod->SetLabel(B_TRANSLATE("Authentication method"));

    mCheckBoxAllowPwdlessLogin = new BCheckBox("cb_pwdless",
        B_TRANSLATE("Allow passwordless accounts to login"),
        new BMessage(M_BOOL_PWDLESS));
    mSliderAttemptsThrshld = new BSlider("sl_att",
        B_TRANSLATE("Count of failed attempts tolerance"),
        new BMessage(M_AUTHOPTS_THRSHD), 0, 10, B_HORIZONTAL, B_TRIANGLE_THUMB,
        B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
    mSliderAttemptsThrshld->SetHashMarks(B_HASH_MARKS_BOTH);
    mSliderAttemptsThrshld->SetHashMarkCount(11);
    mSliderAttemptsThrshld->SetLimitLabels(B_TRANSLATE("0 (limitless)"),
        B_TRANSLATE("10"));
    mSliderErrorWaitTime = new BSlider("sl_snzt",
        B_TRANSLATE("Wait time after too many failed attempts"),
        new BMessage(M_AUTHOPTS_ERRSNZ), 0, 10, B_HORIZONTAL, B_TRIANGLE_THUMB,
        B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
    mSliderErrorWaitTime->SetHashMarks(B_HASH_MARKS_BOTH);
    mSliderErrorWaitTime->SetHashMarkCount(11);
    mSliderErrorWaitTime->SetLimitLabels(B_TRANSLATE("Disabled"),
        B_TRANSLATE("10 seconds"));
    mCheckBoxResetLoginForm = new BCheckBox("cb_resetform",
        B_TRANSLATE("Reset login form if remained inactive for too long time"),
        new BMessage(M_AUTHOPTS_RESETFORM));

    BView* authOptsView = new BView("authview", B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(authOptsView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(mCheckBoxAllowPwdlessLogin)
        .Add(mSliderAttemptsThrshld)
        .Add(mSliderErrorWaitTime)
        .Add(mCheckBoxResetLoginForm)
    .End();


    BBox* mBoxAuthOptions = new BBox("box_authopts",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, authOptsView);
    mBoxAuthOptions->SetLabel(B_TRANSLATE("Authentication options"));

    BView* thisview = new BView("authview", B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(thisview, B_VERTICAL)
        .Add(mBoxAuthMethod)
        .Add(mBoxAuthOptions)
    .End();

    return thisview;
}

BView* mWindow::CreateCardView_User()
{
    /* AddUser */
    mAddUserName = new BTextControl("TextAddUser", B_TRANSLATE("Username"),
        "", new BMessage(TEXTADDUSER));
    const char blacklist [] = {"\0\a\b\t\n\v\f\r\e\x20"};
    for(int i = 0; i < sizeof(blacklist)/sizeof(blacklist[0]); i++)
        mAddUserName->TextView()->DisallowChar(blacklist[i]);
    mAddUserName->SetModificationMessage(new BMessage(CHECK_USERBUTTON));
    mAddPassWord = new BTextControl("TextPassWord", B_TRANSLATE("Password"),
        "", new BMessage(TEXTADDPASS));
    mAddPassWord->SetModificationMessage(new BMessage(CHECK_USERBUTTON));
    mAddPassWord->TextView()->HideTyping(true);
    mAddPassWordRetype = new BTextControl("TextPassWord", B_TRANSLATE("Repeat"),
        "", new BMessage(TEXTADDPASS));
    mAddPassWordRetype->SetModificationMessage(new BMessage(CHECK_USERBUTTON));
    mAddPassWordRetype->TextView()->HideTyping(true);
    mButtonChangeLogin = new BButton("LoginChange", B_TRANSLATE("Change"),
        new BMessage(CHANGE_LOGIN));
    mButtonChangeLogin->SetEnabled(false);

    BView* mView = new BView(BRect(0, 0, 235, 120), "MainView", B_FOLLOW_ALL_SIDES,
        B_WILL_DRAW | B_NAVIGABLE);
    mView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    mView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(mView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGrid()
            .AddTextControl(mAddUserName, 0, 0)
            .AddTextControl(mAddPassWord, 0, 1)
            .AddTextControl(mAddPassWordRetype, 0, 2)
        .End()
        .Add(mButtonChangeLogin)
    .End();

    BBox* mBoxAroundAddUserName = new BBox("box_addusr",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, mView);
    mBoxAroundAddUserName->SetLabel(B_TRANSLATE("Application account"));

    /* UserList */
    mListOfUsers = new BListView(B_SINGLE_SELECTION_LIST);
    mListOfUsers->SetSelectionMessage(new BMessage(M_APPUSER_SEL));
    mListOfUsers->SetInvocationMessage(new BMessage(M_APPUSER_IVK));
    BScrollView* userlistScroll = new BScrollView("scv_usrlst", mListOfUsers,
        B_FOLLOW_LEFT_TOP, B_SUPPORTS_LAYOUT, false, true, B_FANCY_BORDER);

    BStringView* sysaccountsDesc = new BStringView("sv_sysacc",
        B_TRANSLATE("List of user accounts available in the system.\n"
        "A double click shows details of the selected account."));
    BView* mListView = new BView("ListView", B_SUPPORTS_LAYOUT, NULL);
    mListView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    mListView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(mListView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(sysaccountsDesc)
        .Add(userlistScroll)
    .End();

    BBox* mBoxAroundListUsers = new BBox("box_lstusr",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, mListView);
    mBoxAroundListUsers->SetLabel(B_TRANSLATE("Haiku accounts"));

    BView* thisview = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(thisview, B_VERTICAL)
        .Add(mBoxAroundAddUserName)
        .Add(mBoxAroundListUsers)
    .End();

    return thisview;
}

BView* mWindow::CreateCardView_Background()
{
    /* Colorcontrol */
    mCCBgColor = new BColorControl(BPoint(0, 0), B_CELLS_32x8,
        8.5f, "cc_bgcol", new BMessage(M_BGCOLOR_CHANGED), false);
    mButtonDefaultColors = new BButton("DefaultColors",
        B_TRANSLATE("Default"), new BMessage(DEFAULT_COLORS));

    BView* mColorView = new BView("ColorView", B_SUPPORTS_LAYOUT, NULL);
    mColorView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    mColorView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(mColorView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(mCCBgColor)
        .AddGroup(B_HORIZONTAL)
            .AddGlue()
            .Add(mButtonDefaultColors)
        .End()
    .End();

    BBox* mBoxAroundColorControl = new BBox("box_col",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, mColorView);
    mBoxAroundColorControl->SetLabel(B_TRANSLATE("Color"));

    /* ImagePath */

    mPumBgImageOption = new BPopUpMenu("");
    BLayoutBuilder::Menu<>(mPumBgImageOption)
        .AddItem(B_TRANSLATE("Do not use background image"), new BMessage(M_BGMODE_NONE))
        .AddSeparator()
        .AddItem(B_TRANSLATE("Use a single image file"), new BMessage(M_BGMODE_SINGLE))
        .AddItem(B_TRANSLATE("Load images from a folder"), new BMessage(M_BGMODE_FOLDER))
        .AddItem(B_TRANSLATE("Load images from a list file"), new BMessage(M_BGMODE_LIST))
    .End();
    mMfBgImageOption = new BMenuField(
#if(B_HAIKU_VERSION < B_HAIKU_VERSION_1_PRE_BETA_5)
        "mf_bgmode", B_TRANSLATE("Background image mode"), mPumBgImageOption
#else
        "mf_bgmode", B_TRANSLATE("Background image mode"), mPumBgImageOption, true
#endif
    );
    mPumBgImageAdjustment = new BPopUpMenu("");
    BLayoutBuilder::Menu<>(mPumBgImageAdjustment)
        .AddItem(B_TRANSLATE("Keep dimensions and center"), new BMessage(M_BGIMG_ADJUST_KEEP))
        .AddItem(B_TRANSLATE("Scale and fit to X"), new BMessage(M_BGIMG_ADJUST_X))
        .AddItem(B_TRANSLATE("Scale and fit to Y"), new BMessage(M_BGIMG_ADJUST_Y))
        .AddItem(B_TRANSLATE("Scale and fit to X:Y"), new BMessage(M_BGIMG_ADJUST_X_Y))
        .AddItem(B_TRANSLATE("Resize to screen (allows deformation)"), new BMessage(M_BGIMG_ADJUST_STRETCH))
    .End();
    mMfBgImageAdjustment = new BMenuField(
#if(B_HAIKU_VERSION < B_HAIKU_VERSION_1_PRE_BETA_5)
        "mf_bgadj", B_TRANSLATE("Image adjustment"), mPumBgImageAdjustment
#else
        "mf_bgadj", B_TRANSLATE("Image adjustment"), mPumBgImageAdjustment, true
#endif
    );

    mTextControlmPathToImage = new BTextControl("TextPathToFile",
        NULL, "", new BMessage(M_BGIMG_SINGLEPATH));
    mTextControlmPathToImageFolder = new BTextControl("TextPathToImages",
        NULL, "", new BMessage(M_BGIMG_FOLDERPATH));
    mTextControlmPathToImageFolder->SetModificationMessage(new BMessage(CHECK_BUTTONS));
    mTextControlmPathToImageList = new BTextControl("TextPathToList",
        NULL, "", new BMessage(M_BGIMG_LISTPATH));

    mButtonBrowseImageFilePath = new BButton("mFrameButtonBrowseImagePath",
        B_TRANSLATE("Browse"), new BMessage(M_BGIMG_BROWSE_SINGLE));
    mButtonBrowseImageFolderPath = new BButton("mFrameButtonBrowseImageFPath",
        B_TRANSLATE("Browse"), new BMessage(M_BGIMG_BROWSE_FOLDER));
    mButtonBrowseImageListPath = new BButton("mFrameButtonBrowseImageLPath",
        B_TRANSLATE("Browse"), new BMessage(M_BGIMG_BROWSE_LIST));

    mSliderBgSnooze = new BSlider("sl_snooze",
        B_TRANSLATE("Image transition delay (in seconds)"),
        new BMessage(M_BGIMG_SNOOZE_CHANGED), 5, 60, B_HORIZONTAL, B_TRIANGLE_THUMB,
        B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
    mSliderBgSnooze->SetHashMarks(B_HASH_MARKS_BOTH);
    mSliderBgSnooze->SetHashMarkCount(12);
    mSliderBgSnooze->SetKeyIncrementValue(5);
    mSliderBgSnooze->SetLimitLabels(B_TRANSLATE("5"), B_TRANSLATE("60"));

    BView* mImageView = new BView("ImageView", B_SUPPORTS_LAYOUT, NULL);
    mImageView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    mImageView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(mImageView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGrid()
            .AddMenuField(mMfBgImageOption, 0, 0)
            .AddGlue(0, 1)
            .Add(new BStringView(NULL, B_TRANSLATE("Path to image file")), 0, 2)
            .AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING, 1, 2)
                .Add(mTextControlmPathToImage, 0, 0)
                .Add(mButtonBrowseImageFilePath, 1, 0)
            .End()
            .Add(new BStringView(NULL, B_TRANSLATE("Path to image folder")), 0, 3)
            .AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING, 1, 3)
                .Add(mTextControlmPathToImageFolder, 0, 0)
                .Add(mButtonBrowseImageFolderPath, 1, 0)
            .End()
            .Add(new BStringView(NULL, B_TRANSLATE("Path to image list")), 0, 4)
            .AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING, 1, 4)
                .Add(mTextControlmPathToImageList, 0, 0)
                .Add(mButtonBrowseImageListPath, 1, 0)
            .End()
            .AddGlue(0, 5)
            .AddMenuField(mMfBgImageAdjustment, 0, 6)
        .End()
        .Add(mSliderBgSnooze)
    .End();

    BBox* mBoxAroundImagePath = new BBox("box_img",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, mImageView);
    mBoxAroundImagePath->SetLabel(B_TRANSLATE("Background image"));

    /* Layout builder */
    BView* thisview = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(thisview, B_VERTICAL)
        .Add(mBoxAroundColorControl)
        .Add(mBoxAroundImagePath)
        .AddGlue()
    .End();

    return thisview;
}

BView* mWindow::CreateCardView_Clock()
{
    int32 mMinFontValue = 5, mMaxFontValue = 15;
    int32 HashMarksCount = 11;
    const char* mStringSliderMinLimitLabel = B_TRANSLATE_COMMENT("10",
        "The number in your native language");
    const char* mStringSliderMaxLimitLabel = B_TRANSLATE_COMMENT("30",
        "The number in your native language");
    int CurrentSliderValue = 10;

    /* Clock feature enablement */
    mCheckBoxBoolClock = new BCheckBox("Bool clock",
        B_TRANSLATE("Show Clock?"), new BMessage(BOOL_CLOCK),
        B_WILL_DRAW | B_NAVIGABLE);

    /* Clock font size */
    mSliderFontSize = new BSlider("FontSizeSlider", B_TRANSLATE("Clock Size"),
        new BMessage(SIZE_SLIDER_CHANGED), mMinFontValue, mMaxFontValue,
        B_HORIZONTAL, B_TRIANGLE_THUMB, B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
    mSliderFontSize->SetHashMarks(B_HASH_MARKS_BOTH);
    mSliderFontSize->SetHashMarkCount(HashMarksCount);
    mSliderFontSize->SetLimitLabels(mStringSliderMinLimitLabel, mStringSliderMaxLimitLabel);
    mSliderFontSize->SetKeyIncrementValue(2);
    mSliderFontSize->SetValue(CurrentSliderValue);

    /* Clock color */
    mCCClockColor = new BColorControl(BPoint(0, 0), B_CELLS_32x8,
        8.5f, "cc_clockcol", new BMessage(M_CLOCK_COLORCHANGED), false);
    mButtonDefaultClockColors = new BButton("DefaultClockColors",
        B_TRANSLATE("Default"), new BMessage(DEFAULT_CLOCK_COLORS));

    BView* clockColorView = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    clockColorView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    clockColorView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(clockColorView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(mCCClockColor)
        .AddGroup(B_HORIZONTAL)
            .AddGlue()
            .Add(mButtonDefaultClockColors)
        .End()
    .End();

    BBox* boxClockColor = new BBox("box_clock_col",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, clockColorView);
    boxClockColor->SetLabel(B_TRANSLATE("Color"));

    /* Clock placement */
    mTextControlClockPlaceX = new BTextControl("ClockPlacementX",
        B_TRANSLATE("Placement X"), "",
        new BMessage(CLOCKPLACEMENT_X));
    mTextControlClockPlaceX->SetModificationMessage(new BMessage(CHECK_BUTTONS));
    mTextControlClockPlaceY = new BTextControl("ClockPlacementY",
        B_TRANSLATE("Placement Y"), "",
        new BMessage(CLOCKPLACEMENT_Y));
    mTextControlClockPlaceY->SetModificationMessage(new BMessage(CHECK_BUTTONS));
    mButtonDefaultClockPlace = new BButton("DefaultPlace",
        B_TRANSLATE("Default"), new BMessage(DEFAULT_PLACE));

    BView* clockPlaceView = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    clockPlaceView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    clockPlaceView->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(clockPlaceView, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGrid()
            .AddTextControl(mTextControlClockPlaceX, 0, 0)
            .AddTextControl(mTextControlClockPlaceY, 0, 1)
        .End()
        .AddGroup(B_HORIZONTAL)
            .Add(new BStringView(NULL, B_TRANSLATE("Position must be within the screen bounds")))
            .AddGlue()
            .Add(mButtonDefaultClockPlace)
        .End()
    .End();

    BBox* boxClockPlace = new BBox("box_clock_loc",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER, clockPlaceView);
    boxClockPlace->SetLabel(B_TRANSLATE("Placement"));

    BView* thisview = new BView("ClockView", B_SUPPORTS_LAYOUT, NULL);
    thisview->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    thisview->SetHighUIColor(B_PANEL_TEXT_COLOR);

    BLayoutBuilder::Group<>(thisview, B_VERTICAL)
        .SetInsets(0)
        .Add(mCheckBoxBoolClock)
        .Add(mSliderFontSize)
        .Add(boxClockColor)
        .Add(boxClockPlace)
        .AddGlue()
    .End();

    return thisview;
}

BView* mWindow::CreateCardView_Logging()
{
    mCheckBoxEventLog = new BCheckBox("cb_el",
        B_TRANSLATE("Enable login events logging"), new BMessage(M_EVTLOG_BOOL));
    mPumLogLevel = new BPopUpMenu("");
    BLayoutBuilder::Menu<>(mPumLogLevel)
        .AddItem(B_TRANSLATE("Error"), M_EVTLOG_LEVEL_1)
        .AddItem(B_TRANSLATE("Warning"), M_EVTLOG_LEVEL_2)
        .AddItem(B_TRANSLATE("Information"), M_EVTLOG_LEVEL_3)
    .End();
    mMfLogLevel = new BMenuField(
#if(B_HAIKU_VERSION < B_HAIKU_VERSION_1_PRE_BETA_5)
        "mf_loglev", B_TRANSLATE("Log level"), mPumLogLevel
#else
        "mf_loglev", B_TRANSLATE("Log level"), mPumLogLevel, true
#endif
    );

    mPumLogRetentionPolicy = new BPopUpMenu("");
    BLayoutBuilder::Menu<>(mPumLogRetentionPolicy)
        .AddItem(B_TRANSLATE("Continue logging"), M_EVTLOG_POLICY_0)
        .AddItem(B_TRANSLATE("Restart log file if it surpasses certain size"), M_EVTLOG_POLICY_1)
        .AddItem(B_TRANSLATE("Restart log file if it is too old"), M_EVTLOG_POLICY_2)
    .End();

    mMfLogRetentionPolicy = new BMenuField(
#if(B_HAIKU_VERSION < B_HAIKU_VERSION_1_PRE_BETA_5)
        "mf_loglev", B_TRANSLATE("Retention policy"), mPumLogRetentionPolicy
#else
        "mf_loglev", B_TRANSLATE("Retention policy"), mPumLogRetentionPolicy, true
#endif
    );
    mSpinnerLogMaxSize = new BSpinner("sp_size", NULL, new BMessage(M_EVTLOG_POLICY_SIZE));
    mSpinnerLogMaxSize->SetMinValue(1);
    mSpinnerLogMaxAge = new BSpinner("sp_age", NULL, new BMessage(M_EVTLOG_POLICY_AGE));
    mSpinnerLogMaxAge->SetMinValue(1);
    mButtonClearLogs = new BButton("bt_clrlg", B_TRANSLATE("Clear logs"),
        new BMessage(M_EVTLOG_CLEAR));
    mStaticLogClrResponse = new BStringView("sv_rsp", "");

    BView* viewLogRetention = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(viewLogRetention, B_VERTICAL)
        .SetInsets(B_USE_SMALL_INSETS)
        .AddGrid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
            .AddMenuField(mMfLogRetentionPolicy, 0, 0)
            .Add(new BStringView("sv_size", B_TRANSLATE("Max. size (MiB)")), 0, 1)
            .Add(mSpinnerLogMaxSize, 1, 1)
            .Add(new BStringView("sv_age", B_TRANSLATE("Max. age (days)")), 0, 2)
            .Add(mSpinnerLogMaxAge, 1, 2)
        .End()
        .AddGroup(B_HORIZONTAL)
            .Add(mButtonClearLogs)
            .AddGlue()
            .Add(mStaticLogClrResponse)
        .End()
    .End();
    BBox* boxLogRetention = new BBox("box_boot",
        B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
        B_FANCY_BORDER, viewLogRetention);
    boxLogRetention->SetLabel(B_TRANSLATE("Log retention"));

    BView* thisview = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(thisview, B_VERTICAL)
        .SetInsets(0)
        .Add(mCheckBoxEventLog)
        .Add(mMfLogLevel)
        .Add(boxLogRetention)
        .AddGlue()
    .End();

    return thisview;
}

BView* mWindow::CreateCardView_Options()
{
    mCheckBoxSessionBar = new BCheckBox("cb_sb",
        B_TRANSLATE("Show session bar (shutdown, restart)"), new BMessage(M_BOOL_SESSION));
    mCheckBoxSysInfo = new BCheckBox("cb_si",
        B_TRANSLATE("Show system information panel"), new BMessage(M_BOOL_INFO));
    mCheckBoxKillerShortcut = new BCheckBox("cb_ks",
        B_TRANSLATE("Enable shortcut (Cmd+Ctrl+Space) to bypass password protection"),
        new BMessage(M_BOOL_KILLER));

    mButtonAddToBoot = new BButton(NULL, B_TRANSLATE("Add to boot"),
        new BMessage(M_UTIL_ADDBOOT));
    mButtonRemFromBoot = new BButton(NULL, B_TRANSLATE("Remove from boot"),
        new BMessage(M_UTIL_REMBOOT));

    BView *view = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(view, B_HORIZONTAL)
        .SetInsets(B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
        .Add(mButtonAddToBoot)
        .Add(mButtonRemFromBoot)
    .End();

    BBox* box = new BBox("box_boot", B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE_JUMP,B_FANCY_BORDER,view);
    box->SetLabel(B_TRANSLATE("Startup"));

    BView* thisview = new BView(NULL, B_SUPPORTS_LAYOUT, NULL);
    BLayoutBuilder::Group<>(thisview, B_VERTICAL)
        .Add(mCheckBoxSessionBar)
        .Add(mCheckBoxSysInfo)
        .Add(mCheckBoxKillerShortcut)
        .Add(box)
        .AddGlue()
    .End();

    return thisview;
}

// #pragma mark - Checks

bool mWindow::UI_IsDefault(BMessage* defaults)
{
    return
           (mRadioBtAuthAppaccount->Value() == B_CONTROL_ON && AUTH_APP_ACCOUNT == defaults->GetUInt8(mNameConfigAuthMode, AUTH_APP_ACCOUNT)) &&
           (mCheckBoxAllowPwdlessLogin->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigPwdLessLogonOn, true) &&
           (mSliderAttemptsThrshld->Value() == defaults->GetInt32(mNameConfigAuthAttemptsThrshld, 0)) &&
           (mSliderErrorWaitTime->Value()  == defaults->GetInt32(mNameConfigAuthSnoozeAfterErrors, 5)) &&
           (mCheckBoxResetLoginForm->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigAuthResetForm, false) &&
           (mMfBgImageOption->Menu()->ItemAt(defaults->GetUInt8(mNameConfigBgMode, BGM_NONE))->IsMarked()) &&
           (UI_IsBgColorDefault(defaults)) &&
           // (strcmp(mTextControlmPathToImage->Text(), "") == 0) &&
           // (UI_IsBgFolderDefault(defaults)) &&
           // (strcmp(mTextControlmPathToImageList->Text(), "") == 0) &&
           (mSliderBgSnooze->Value() == defaults->GetUInt32(mNameConfigBgSnooze, 10)) &&
           (mMfBgImageAdjustment->Menu()->ItemAt(defaults->GetUInt8(mNameConfigImageAdjustment, BGI_ADJ_SCALE_X_Y))->IsMarked()) &&
           (UI_IsClockColorDefault(defaults)) &&
           (UI_IsClockPlaceDefault(defaults)) &&
           (mCheckBoxBoolClock->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigBoolClock, true) &&
           (mSliderFontSize->Value() == defaults->GetUInt32(mNameConfigClockFontSize, 8)) &&
           (mCheckBoxEventLog->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigEvtLoggingOn, true) &&
           (mMfLogLevel->Menu()->ItemAt(EVT_INFO - 1)->IsMarked()) &&
           (mMfLogRetentionPolicy->Menu()->ItemAt(defaults->GetUInt8(mNameConfigEvtLoggingRetention, EVP_CONTINUE))->IsMarked()) &&
           (mSpinnerLogMaxSize->Value() == defaults->GetUInt32(mNameConfigEvtLoggingMaxSize, 1)) &&
           (mSpinnerLogMaxAge->Value() == defaults->GetUInt32(mNameConfigEvtLoggingMaxAge, 1)) &&
           (mCheckBoxSessionBar->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigSessionBarOn, false) &&
           (mCheckBoxSysInfo->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigSysInfoPanelOn, true) &&
           (mCheckBoxKillerShortcut->Value() == B_CONTROL_ON) == defaults->GetBool(mNameConfigKillerShortcutOn, false);
}

bool mWindow::UI_IsBgColorDefault(BMessage* archive)
{
    return mCCBgColor->ValueAsColor() ==
        archive->GetColor(mNameConfigBgColor, {});
}

bool mWindow::UI_IsBgFolderDefault(BMessage* archive)
{
/*
    if(strcmp(mTextControlmPathToImageFolder->Text(), "") == 0)
        return false;
    else
        return strcmp(mTextControlmPathToImageFolder->Text(),
            archive->GetString(mNameConfigImagePath)) == 0;
*/
    return true;
}

bool mWindow::UI_IsClockColorDefault(BMessage* archive)
{
    return mCCClockColor->ValueAsColor() ==
        archive->GetColor(mNameConfigClockColor, {});
}

bool mWindow::UI_IsClockPlaceDefault(BMessage* archive)
{
    return
        atoi(mTextControlClockPlaceX->Text()) == archive->GetPoint(mNameConfigClockPlace, {}).x &&
        atoi(mTextControlClockPlaceY->Text()) == archive->GetPoint(mNameConfigClockPlace, {}).y;
}

void mWindow::CallBgImgFilePanel(uint32 what, uint32 node_flavors, BRefFilter* f)
{
    BMessage* notifymsg = new BMessage(what);

    mFilePanelFolderBrowse->SetMessage(notifymsg);
    mFilePanelFolderBrowse->SetNodeFlavors(node_flavors);
    if(f != NULL)
        mFilePanelFolderBrowse->SetRefFilter(f);

    mFilePanelFolderBrowse->Show();
    delete notifymsg;
}

void mWindow::EnableAutoStart(bool status)
{
    BPath path;
    find_directory(B_USER_BOOT_DIRECTORY, &path);
    path.Append("launch", true);

    if(status && HasAutoStartInstalled() == B_ENTRY_NOT_FOUND) {
        entry_ref ref;
        be_roster->FindApp("application/x-vnd.LockWorkstation", &ref);
        BEntry entry(&ref);
        BPath binpath;
        entry.GetPath(&binpath);
        BSymLink symlink(&ref);
        BDirectory autolaunch(path.Path());
        autolaunch.CreateSymLink(ref.name, binpath.Path(), &symlink);
    }
    else {
        BDirectory directory(path.Path());
        BEntry entry(&directory, "LockWorkstation", false);
        if(HasAutoStartInstalled() == B_BAD_DATA) {
            int32 result = ((new BAlert(B_TRANSLATE("Entry exists"),
                B_TRANSLATE("There is currently an entry that does not point "
                "to the right destination or it is not a symbolic link.\nDo "
                "you want to delete it anyways?"),
                B_TRANSLATE("Delete file"),
                B_TRANSLATE("Keep existing entry"))))->Go();
            if(result == 0) {
                entry.Remove();
            }
        }
        else {
            entry.Remove();
        }
    }
}

status_t mWindow::HasAutoStartInstalled(const char* targetname)
{
    BPath path;
    find_directory(B_USER_BOOT_DIRECTORY, &path);
    path.Append("launch", true);
    BDirectory launchdir(path.Path());

    BEntry entry(&launchdir, targetname, false);
    if(entry.Exists()) { // match found...
        if(!entry.IsSymLink()) // but it is not a symlink as it should have been
            return B_BAD_DATA;
        else {
            BSymLink symlink(&entry);
            char buffer[B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH];
            if(symlink.ReadLink(buffer, B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH) > 0) {
                BFile file(buffer, B_READ_ONLY);
                BAppFileInfo fileinfo(&file);
                char signature[B_MIME_TYPE_LENGTH];
                fileinfo.GetSignature(signature);
                if(strcmp(signature, "application/x-vnd.LockWorkstation") == 0)
                    return B_OK;
                else
                    return B_ERROR;
            }
            else
                return B_BAD_DATA;

        }
    }
    else // entry does not exist, we are free to create one without issue
        return B_ENTRY_NOT_FOUND;
}

void mWindow::UI_ClockControlsEnable(BMessage* defaults, bool status)
{
    mSliderFontSize->SetEnabled(status);
    mCCClockColor->SetEnabled(status);
    mButtonDefaultClockColors->SetEnabled(status ? !UI_IsClockColorDefault(defaults) : false);
    mTextControlClockPlaceX->SetEnabled(status);
    mTextControlClockPlaceY->SetEnabled(status);
    mButtonDefaultClockPlace->SetEnabled(status ? !UI_IsClockPlaceDefault(defaults) : false);
}

void mWindow::UI_LogControlsEnable(bool status)
{
    auto c = [=](bool s, bool p) {
        if(s && p) return tint_color(ui_color(B_PANEL_TEXT_COLOR), B_NO_TINT);
        else return tint_color(ui_color(B_PANEL_TEXT_COLOR), (0.590f + 0.385f) * (4.0f / 7));
    };
    bool polSize = mMfLogRetentionPolicy->Menu()->ItemAt(EVP_WIPE_AFTER_SIZE)->IsMarked();
    bool polAge = mMfLogRetentionPolicy->Menu()->ItemAt(EVP_WIPE_AFTER_AGE)->IsMarked();

    mMfLogLevel->SetEnabled(status);
    mMfLogRetentionPolicy->SetEnabled(status);

    FindView("sv_size")->SetHighColor(c(status, polSize));
    FindView("sv_age")->SetHighColor(c(status, polAge));
    FindView("sv_size")->Invalidate();
    FindView("sv_age")->Invalidate();

    mSpinnerLogMaxSize->SetEnabled(status && polSize);
    mSpinnerLogMaxAge->SetEnabled(status && polAge);
}
