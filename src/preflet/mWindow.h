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

#define TEXTADDUSER 'pro2'
#define TEXTADDPASS 'pro3'
#define USERLIST 'pro4'
#define TEXT_IMAGEPATH 'pro5'
#define BUTTON_DEFAULTPATH 'pro9'
#define BUTTON_BROWSEPATH 'prox'
#define COLOR_CHANGED_R 'prx4'
#define COLOR_CHANGED_G 'prx5'
#define COLOR_CHANGED_B 'prx6'
#define ERASER_FROM_DOOM 'prx8'
#define BUTTON_CLICKED 'pr21'
#define QUIT_TEAM_MONITOR 'Dsab'
#define CLOCKCOLOR_CHANGED_R 'eres'
#define CLOCKCOLOR_CHANGED_G 'ere2'
#define CLOCKCOLOR_CHANGED_B 'ere3'
#define BOOL_CLOCK 'ere5'
#define CLOCKPLACEMENT_X 'ere6'
#define CLOCKPLACEMENT_Y 'ere7'
#define APPLY_EVERYTHING 'ere9'
#define DEFAULT_COLORS 'erea'
#define DEFAULT_CLOCK_COLORS 'eraa'
#define DEFAULT_PLACE 'asda'
#define FOLDER_SELECTED 'asds'
#define SIZE_SLIDER_CHANGED 'asdy'
#define CHANGE_LOGIN 'asd4'
#define BROWSE_FOLDER 'asd3'
#define FOLDER_CHANGED 'a333'
#define LANG_0 'aaaa'
#define LANG_1 'aann'
#define LANG_2 'aan1'
#define LANG_3 'aan2'
#define LANG_4 'aan3'
#define LANG_5 'aan4'
#define LANG_6 'aan5'
#define CHECK_BUTTONS           'ada2'
#define CHECK_USERBUTTON        'wada'
#define M_ITEM_SELECTED         'isel'
#define M_SNOOZE_SLIDER_CHANGED 'snzs'
#define M_BGMODE_RADIO_NONE     'rad0'
#define M_BGMODE_RADIO_FOLDER   'rad1'
#define M_BGMODE_RADIO_LIST     'rad2'
#define M_BOOL_SESSION          'cbsb'
#define M_BOOL_INFO             'cbsi'
#define M_BOOL_KILLER           'cbks'
#define M_BOOL_EVTLOG           'cbel'
#define M_AUTHMTHD_SYSTEM       'aths'
#define M_AUTHMTHD_KEYSTR       'athk'
#define M_AUTHMTHD_APPACC       'atha'
#define M_APPUSER_MODIFY        'usmd'
#define M_APPUSER_REMOVE        'usrm'
#define M_APPUSER_SEL           'usel'
#define M_APPUSER_IVK           'uivk'

#include <StorageKit.h>
#include <KernelKit.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <Spinner.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mBitmap.h"
#include "../common/LockWorkstationConfig.h"
#include "../common/ThreadedClass.h"

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
                       *mButtonBrowseImagePath,
                       *mButtonDefaultClockColors,
                       *mButtonDefaultClockPlace,
                       *mButtonApplyEverything,
                       *mEraserButtonOfDoom;
    BCardView          *fCardView;
	BCheckBox          *mCheckBoxBoolClock,
                       *mCheckBoxSessionBar,
	                   *mCheckBoxSysInfo,
	                   *mCheckBoxKillerShortcut,
	                   *mCheckBoxEventLog;
	BListView          *mListOfUsers,
	                   *fPanelList;
    BRadioButton       *mRadioBtAuthSysaccount,
                       *mRadioBtAuthSyskeystore,
                       *mRadioBtAuthAppaccount,
                       *mRadioBtUseBgImgNone,
                       *mRadioBtUseBgImgFolder,
                       *mRadioBtUseBgImgList;
	BSlider            *mSliderFontSize,
                       *mSliderBgSnooze;
	BSpinner           *mSpinnerColorR,
	                   *mSpinnerColorG,
	                   *mSpinnerColorB,
	                   *mSpinnerClockColorR,
	                   *mSpinnerClockColorG,
	                   *mSpinnerClockColorB;
	BTextControl       *mAddUserName,
	                   *mAddPassWord,
	                   *mAddPassWordRetype,
	                   *mTextControlmPathToImageFolder,
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
};

#endif
