// mApp.cpp
#include <cstdio>
#include <Application.h>
#include "mApp.h"

int main(int argc, char** argv)
{
    mApp *app = new mApp();
    app->Run();
    delete app;
    return 0;
}

mApp::mApp()
: BApplication(ApplicationSignature)
{
    MainWindow = new mWindow("Main Window");
}

mApp::~mApp()
{
}

void mApp::ReadyToRun()
{
    if (MainWindow != NULL)
        MainWindow->Show();
    else {
        fprintf(stderr, "Something went wrong, closing app...\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
    }
}
