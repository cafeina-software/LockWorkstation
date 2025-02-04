/**********************************************************************************/
/*****************        Lock Workstation By Marius Stene        *****************/
/*****************             m_stene@yahoo.com, 2002            *****************/
/*****************        My contribution to this great OS        *****************/
/**********************************************************************************/
/*****************  You are welcome to use any part of this code  *****************/
/*****************      in whole, or in parts freely without      *****************/
/*****************                  contacting me                 *****************/
/**********************************************************************************/

#ifndef _mWindow_h_
#define _mWindow_h_

#include <StorageKit.h>
#include <KernelKit.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <CardView.h>
#include <private/interface/Spinner.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mBitmap.h"
#include "mConstant.h"
#include "mUserInfo.h"
#include "../common/LockWorkstationConfig.h"
#include "../common/ThreadedClass.h"

class ListFilter : public BRefFilter
{
public: virtual bool Filter(const entry_ref* ref, BNode* node,
    struct stat_beos* stat, const char* mimetype) {
        return (node->IsDirectory()) ||
               (strcmp(mimetype, "text/plain") == 0);
    }
};
class ImageFilter : public BRefFilter
{
public: virtual bool Filter(const entry_ref* ref, BNode* node,
    struct stat_beos* stat, const char* mimetype) {
        return (node->IsDirectory()) ||
               (strstr(mimetype, "image") != NULL) ||
               (strstr(ref->name, ".jpg") != NULL) ||
               (strstr(ref->name, ".png") != NULL) ||
               (strstr(ref->name, ".bmp") != NULL);
    }
};

class mWindow
		: public BWindow, public ThreadedClass
{
public:
						mWindow(const char *mWindowTitle);
						~mWindow();
	virtual void 		MessageReceived(BMessage* message);
	virtual bool 		QuitRequested();
private:
	//Thread
	static int32 		EnDButtonsThread_static(void *data);
	void 				EnDButtons_Thread();
	static int32 		EnDUserButtonThread_static(void *data);
	void 				EnDUserButton_Thread();
    static int32        UpdateUIThread_static(void* data);
    void                UpdateUI_Thread();

    void                InitUIControls();
    void                InitUIData();
	status_t			DisplayCard(int32 index);
    void                CallBgImgFilePanel(uint32 what, uint32 node_flavors,
                            BRefFilter* filter = NULL);
    void                EnableAutoStart(bool status);
    status_t            HasAutoStartInstalled(const char* targetname = "LockWorkstation");

    //UI
    BView*              CreateCardView_AccountMethod();
	BView*              CreateCardView_User();
	BView*              CreateCardView_Background();
	BView*              CreateCardView_Clock();
    BView*              CreateCardView_Logging();
	BView*              CreateCardView_Options();

	bool				UI_IsDefault(BMessage* archive);
	bool				UI_IsBgColorDefault(BMessage* archive);
	bool				UI_IsBgFolderDefault(BMessage* archive);
	bool				UI_IsClockColorDefault(BMessage* archive);
	bool				UI_IsClockPlaceDefault(BMessage* archive);

    void                UI_ClockControlsEnable(BMessage* defaults, bool status);
    void                UI_LogControlsEnable(bool status);
private:
    BView              *amthdCardView,
                       *userCardView,
                       *bgCardView,
                       *clockCardView,
                       *logCardView,
                       *extraCardView,
                       *mApplyView;

    BButton            *mButtonChangeLogin,
                       *mButtonUserMod,
                       *mButtonUserRem,
                       *mButtonDefaultColors,
                       *mButtonDefaultImagePath,
                       *mButtonBrowseImageFilePath,
                       *mButtonBrowseImageFolderPath,
                       *mButtonBrowseImageListPath,
                       *mButtonDefaultClockColors,
                       *mButtonDefaultClockPlace,
                       *mButtonApplyEverything,
                       *mButtonClearLogs,
                       *mButtonAddToBoot,
                       *mButtonRemFromBoot,
                       *mEraserButtonOfDoom;
    BCardView          *fCardView;
    BCheckBox          *mCheckBoxBoolClock,
                       *mCheckBoxSessionBar,
                       *mCheckBoxSysInfo,
                       *mCheckBoxKillerShortcut,
                       *mCheckBoxEventLog,
                       *mCheckBoxResetLoginForm,
                       *mCheckBoxAllowPwdlessLogin;
    BColorControl      *mCCBgColor,
                       *mCCClockColor;
    BListView          *mListOfUsers,
                       *fPanelList;
    BMenuField         *mMfBgImageOption,
                       *mMfBgImageAdjustment,
                       *mMfLogLevel,
                       *mMfLogRetentionPolicy;
    BPopUpMenu         *mPumBgImageOption,
                       *mPumBgImageAdjustment,
                       *mPumLogLevel,
                       *mPumLogRetentionPolicy;
    BRadioButton       *mRadioBtAuthSysaccount,
                       *mRadioBtAuthAppaccount;
    BSlider            *mSliderFontSize,
                       *mSliderAttemptsThrshld,
                       *mSliderErrorWaitTime,
                       *mSliderBgSnooze;
    BSpinner           *mSpinnerLogMaxSize,
                       *mSpinnerLogMaxAge;
    BStringView        *mStaticLogClrResponse;
    BTextControl       *mAddUserName,
                       *mAddPassWord,
                       *mAddPassWordRetype,
                       *mTextControlmPathToImage,
                       *mTextControlmPathToImageFolder,
                       *mTextControlmPathToImageList,
                       *mTextControlClockPlaceX,
                       *mTextControlClockPlaceY;

    thread_id           EnDButtonsThread,
                        EnDUserButtonThread,
                        UpdateUIThread;

    LWSettings         *settings;

    BFilePanel*         mFilePanelFolderBrowse;
    entry_ref           mEntryRef;
    mUserInfo          *fInfoWnd;
};

#endif
