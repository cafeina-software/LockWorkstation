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

// #pragma mark - main

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "(preflet) main"

int main(int argc, char** argv)
{
    if(argc > 1) {
        switch(option(argv[1]))
        {
            case 1:
                help();
                return 0;
            case 2:
                printf(B_TRANSLATE("%s: version %s\n"),
                    B_TRANSLATE_SYSTEM_NAME(ApplicationName), ApplicationVersion);
                return 0;
            case 3:
                return create_default_config(true);
            case 4:
                return create_default_config(false);
            default:
                break;
        }
    }
    mApp* app = new mApp();
    app->Run();
    delete app;
    return(0);
}

int option(const char* op)
{
    if(strcmp(op, "--help") == 0)
        return 1;
    else if(strcmp(op, "--version") == 0)
        return 2;
    else if(strcmp(op, "--restore-settings") == 0)
        return 3;
    else if(strcmp(op, "--create-settings") == 0)
        return 4;
    else
        return 0;
}

void help()
{
    BString description(B_TRANSLATE(
        "%appname%: %appdesc%\n"
        "Usage: %binname%\n"
        "       %binname% [option]\n"
        "The first mode opens the UI preferences window.\n"
        "The second mode perform an operation and exits. Here, [option] includes:\n"
        "\t--help              This message\n"
        "\t--version           Shows application version\n"
        "\t--create-settings   Creates a new settings file if it does not exist\n"
        "\t--restore-settings  Removes the settings file and creates a new one\n"
    ));
    description.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(ApplicationName));
    description.ReplaceAll("%appdesc%", B_TRANSLATE(ApplicationDescription));
    description.ReplaceAll("%binname%", ApplicationBinName);

    printf("%s", description.String());
}

status_t create_default_config(bool eraseIfExists)
{
    BPath path;
    if(find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK) {
        printf(B_TRANSLATE("Error: user settings directory could not be fetched.\n"));
        return B_ERROR;
    }

    path.Append("LockWorkstationSettings", true);
    BEntry entry(path.Path());
    if(entry.Exists()) {
        if(!eraseIfExists) {
            printf(B_TRANSLATE("Error: settings file %s already exists.\n"), path.Path());
            return B_FILE_EXISTS;
        }
        else
            entry.Remove();
    }

    BMessage* defaults = new BMessage;
    LWSettings::DefaultSettings(defaults);
    BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
    if(file.InitCheck() != B_OK) {
        printf(B_TRANSLATE("Error: settings file %s could not be opened.\n"), path.Path());
        delete defaults;
        return B_ERROR;
    }
    defaults->Flatten(&file);

    delete defaults;
    return B_OK;
}

// #pragma mark - mApp

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
    MainWindow = new mWindow(B_TRANSLATE_SYSTEM_NAME(ApplicationName));
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
#define B_TRANSLATION_CONTEXT "(preflet) About box"

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

    BAboutWindow* about = new BAboutWindow(B_TRANSLATE_SYSTEM_NAME(ApplicationName),
        ApplicationSignature);
    about->SetVersion(ApplicationVersion);
    about->AddDescription(B_TRANSLATE(ApplicationDescription));
    about->AddCopyright(2002, "Marius Stene", extraCopyrights);
    about->AddAuthors(authors);
    // about->AddSpecialThanks(NULL);
    // about->AddVersionHistory(NULL);
    about->AddExtraInfo(B_TRANSLATE("\xe2\x80\x94\"Don't say a word!\"\n\xe2\x80\x94\"OK\""));
    about->AddText(B_TRANSLATE("Website"), website);
    about->AddText(B_TRANSLATE("E-Mail"), email);
    about->Show();
}
