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
    void                AppUserMod(const char* n, const char* p, const char* nn = NULL);
private:
	//Thread
	static int32 		CheckerThread_static(void *data);
	void 				Checker_Thread();
	static int32 		EnDButtonsThread_static(void *data);
	void 				EnDButtons_Thread();
	static int32 		EnDUserButtonThread_static(void *data);
	void 				EnDUserButton_Thread();
	static int32 		UpdateStringsThread_static(void *data);
	void 				UpdateStrings_Thread();

    void                InitUIControls();
    void                InitUIData();
    void                CallBgImgFilePanel(uint32 what, uint32 node_flavors,
                            BRefFilter* filter = NULL);
    void                EnableAutoStart(bool status);
    status_t            HasAutoStartInstalled(const char* targetname = "LockWorkstation");

    //UI
    BView*              CreateCardView_AccountMethod();
	BView*              CreateCardView_User();
	BView*              CreateCardView_Background();
	BView*              CreateCardView_Clock();
	BView*              CreateCardView_Options();

	bool				UI_IsDefault(BMessage* archive);
	bool				UI_IsBgColorDefault(BMessage* archive);
	bool				UI_IsBgFolderDefault(BMessage* archive);
	bool				UI_IsClockColorDefault(BMessage* archive);
	bool				UI_IsClockPlaceDefault(BMessage* archive);
private:
	BView              *amthdCardView,
                       *userCardView,
	                   *bgCardView,
                       *clockCardView,
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
                       *mCheckBoxAllowPwdlessLogin;
    BColorControl      *mCCBgColor,
                       *mCCClockColor;
	BListView          *mListOfUsers,
	                   *fPanelList;
    BMenuField         *mMfBgImageOption;
    BPopUpMenu         *mPumBgImageOption;
    BRadioButton       *mRadioBtAuthSysaccount,
                       *mRadioBtAuthAppaccount;
    BSlider            *mSliderFontSize,
                       *mSliderAttemptsThrshld,
                       *mSliderErrorWaitTime,
                       *mSliderBgSnooze;
    BStringView        *mStaticLogClrResponse;
    BTextControl       *mAddUserName,
	                   *mAddPassWord,
	                   *mAddPassWordRetype,
                       *mTextControlmPathToImage,
	                   *mTextControlmPathToImageFolder,
                       *mTextControlmPathToImageList,
	                   *mTextControlClockPlaceX,
                       *mTextControlClockPlaceY;

    thread_id           CheckerThread,
                        EnDButtonsThread,
                        EnDUserButtonThread,
                        UpdateStringsThread;

    LWSettings         *settings;

	//BBitmap*			BitmapBounds;
	//BBitmap*			BitmapBounds2;
	//BBitmap*			BitmapBounds3;

	BFilePanel*			mFilePanelFolderBrowse;
	entry_ref 			mEntryRef;
    mUserInfo          *fInfoWnd;
};

#endif
