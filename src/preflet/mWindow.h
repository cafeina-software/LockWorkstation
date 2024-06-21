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
#include "../common/ThreadedClass.h"

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
    void                ArchiveData(BMessage* archive);

    //UI
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
	BView              *userCardView,
	                   *bgCardView,
                       *clockCardView,
                       *extraCardView,
                       *mApplyView;

	BButton            *mButtonChangeLogin,
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
    BRadioButton       *mRadioBtUseBgImgNone,
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

	BMessage 			savemessage;
	BString				mStringUser1,
                        mStringPassword1,
                        mStringCurrentPathImages,
                        mStringLanguage;
    rgb_color           mColorBackground,
                        mColorClock;
    uint8               mUintBgMode;
    uint32              mUintSnoozeMultiplier,
                        mUintClockSize;
    BPoint              mPointClockPlace;
    bool                mBoolClockShown,
                        mBoolSessionBarOn,
                        mBoolSysInfoPanelOn,
                        mBoolKillerShortcutOn,
                        mBoolLoggingOn;

    int					LanguageNumber;
	int32				temp, tempHack;
	char 				charer[1024];
	BString 			PathToBG;
	BString 			PathToUI;
	BString 			PathToNOUI;
	BString				mStringClockFontSize;
	BString				mStringInstallDir;
	BString				mDefaultImagePathText;
	BString             mDefaultImagepathTextHack,
                        mDefaultColorRHack,
                        mDefaultColorGHack,
                        mDefaultColorBHack,
                        mDefaultClockColorRHack,
                        mDefaultClockColorGHack,
                        mDefaultClockColorBHack,
                        mDefaultClockPlaceXHack,
                        mDefaultClockPlaceYHack,
                        mDefaultBoolClockHack,
                        mPasswordHack,
                        mPasswordRetypeHack,
                        mPasswordDisableButtonHack;

	BStringItem*		mStringItemUser1;

	//BBitmap*			BitmapBounds;
	//BBitmap*			BitmapBounds2;
	//BBitmap*			BitmapBounds3;

	BMenu*				mMenuLanguage;
	BMenuField*			mMenuFieldLanguage;
	BPopUpMenu*			mPopUpMenuLanguage;
	BMenuItem*			mMenuItem;
	BFilePanel*			mFilePanelFolderBrowse;
	entry_ref 			mEntryRef;
	BPath 				path;
	status_t			status;
	BFile				file;

    BString pwd;
};

#endif
