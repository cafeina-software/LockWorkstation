/**********************************************************************************/
/*****************        Lock Workstation By Marius Stene        *****************/
/*****************             m_stene@yahoo.com, 2002            *****************/
/*****************        My contribution to this great OS        *****************/
/**********************************************************************************/
/*****************  You are welcome to use any part of this code  *****************/
/*****************      in whole, or in parts freely without      *****************/
/*****************                  contacting me                 *****************/
/**********************************************************************************/
/*
************                                             ************                *********************
**************                                        ******************            ***********************
******     *****                                    ****              ****         *****                 ***
*****       *****                                  ****                ****       *****
*****        *****                                ****                  ****      *****
*****        *****                               ****                    ****     *****
*****        ****                                ****                    ****     *****
******    ******                                ****                      ****     **************
***************                ****             ****                      ****      **********************
***************            ************         ****                      ****                  ***********
******    *****          *****      *****       ****                      ****                         *****
*****       *****      *****         *****       ****                    ****                           *****
*****        *****     *******************       ****                    ****                           ****
*****        *****     *******************        ****                  ****                           ****
*****        ****      *****                       ****                ****                           ****
******      *****       ******        ***           ****              ****        ***                ****
***************          ***************              ******************           *********************
*************              ***********                   ************                ******************
*/
#include <iostream>
#include <stdio.h>
#include <private/interface/AboutWindow.h>
#include <Application.h>
#include <Catalog.h>
#include "mApp.h"

//main(int argc, char* argv[]) < argv[0]
int main(int32 argc, char** argv)
{
    mApp* app = new mApp();
    //Path to exe file
    BString				mPathToExecFile;
    BPath 				path;
    status_t			status;
    BFile				file;
    BMessage 			savemessage;

    mPathToExecFile.SetTo(argv[0]);
    mPathToExecFile.RemoveLast("Preferences");
    find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    path.Append("LockWorkstationSettings");
    status = file.SetTo(path.Path(), B_READ_ONLY);
    if (status == B_OK) {
        status = savemessage.Unflatten(&file);
    }
    savemessage.ReplaceString("execDir", mPathToExecFile);
    if (file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE) == B_OK) {
        savemessage.Flatten(&file);
    }
    app->Run();
    delete app;
    return(0);
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "(preflet) mApp"

mApp::mApp()
		: BApplication(ApplicationSignature)
{
}

mApp::~mApp()
{
}

void mApp::ReadyToRun()
{
    MainWindow = new mWindow(B_TRANSLATE_SYSTEM_NAME("LockWorkstation Preferences"));
    if (MainWindow != NULL)
        MainWindow->Show();
    else
		be_app->PostMessage(B_QUIT_REQUESTED);
}

void mApp::MessageReceived(BMessage* message)
{
    switch(message->what)
    {
        case B_ABOUT_REQUESTED:
            AboutRequested();
            break;
        default:
            BApplication::AboutRequested();
            break;
    }
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "About box"

void mApp::AboutRequested()
{
    const char* authors [] = {
        "Marius Stene",
        "Cafeina",
        NULL
    };
    const char* extraCopyrights [] = {
        "2024 Cafeina",
        NULL
    };
    const char* website [] = {
        ApplicationWebsite,
        ApplicationWebsiteOld,
        NULL
    };
    const char* email [] = {
        ApplicationAuthorEMail,
        NULL
    };

    BAboutWindow* about = new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("Lock Workstation Preferences"),
        ApplicationSignature);
    about->SetVersion(ApplicationVersion);
    about->AddDescription(B_TRANSLATE("A simple screen locker for the Haiku operating system."));
    about->AddCopyright(2002, "Marius Stene", extraCopyrights);
    about->AddAuthors(authors);
    // about->AddSpecialThanks(NULL);
    // about->AddVersionHistory(NULL);
    about->AddExtraInfo(B_TRANSLATE("\xe2\x80\x94\"Don't say a word!\"\n\xe2\x80\x94\"OK\""));
    about->AddText(B_TRANSLATE("Website"), website);
    about->AddText(B_TRANSLATE("E-Mail"), email);
    about->Show();
}
