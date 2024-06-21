/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "mBackgroundView.h"

BBitmap* mBackgroundView::currentimage = nullptr;
bool mBackgroundView::shouldExit = false;
static uint32 multiplier;

mBackgroundView::mBackgroundView(BRect frame, const char *name, uint32 resizingMode,
    uint32 flags, rgb_color _bgcolor, uint8 bgmode, BString imgfolder, uint32 snooze)
: BView(frame, name, resizingMode, flags),
  fBackgroundColor(_bgcolor),
  fBackgroundMode(bgmode),
  imageFolderPath(imgfolder),
  snoozeMultiplier(snooze)
{
    SetDrawingMode(B_OP_OVER);
    SetViewColor(fBackgroundColor);

    InitUIData();
}

mBackgroundView::~mBackgroundView()
{
    shouldExit = true;
    kill_thread(imageLooper);
}

void mBackgroundView::Draw(BRect updateRect)
{
    BView::Draw(updateRect);

    if(currentimage != nullptr)
        DrawBitmap(currentimage, BPoint(0, 0));

    Invalidate();
}

int mBackgroundView::CallSetBackgroundImage(void* data)
{
    mBackgroundView* view = (mBackgroundView*) data;
    view->SetBackgroundImage(&view->imagePaths);
    return 0;
}

void mBackgroundView::SetBackgroundImage(void* data)
{
    BStringList* localList = (BStringList*)data;
    while(!shouldExit) {
        for(int i = 1; i < localList->CountStrings(); i++){
            BBitmap* image = BTranslationUtils::GetBitmap(localList->StringAt(i));
            if(image != NULL) {
                snooze(multiplier * 1000000);
                currentimage = image;
            }
        }
    }
}

void mBackgroundView::InitUIData()
{
    multiplier = snoozeMultiplier;

    if(fBackgroundMode && !imageFolderPath.IsEmpty()) {
        if(InitImageList() == B_OK) {
            /* Initial image */
            currentimage = BTranslationUtils::GetBitmap(imagePaths.StringAt(0));

            /* Image loop */
            ThreadedCall(imageLooper, &mBackgroundView::CallSetBackgroundImage,
                "image loop", B_DISPLAY_PRIORITY, this);
        }
    }
}

status_t mBackgroundView::InitImageList()
{
    /* Check whether it is or not a valid directory */
    BDirectory directory(imageFolderPath.String());
    if(directory.InitCheck() != B_OK)
        return B_NOT_A_DIRECTORY;

    /* Gather the directory's contents (ignoring any permission denied cases) */
    status_t status;
    BEntry entry;
    while((status = directory.GetNextEntry(&entry, true)) != B_ENTRY_NOT_FOUND) {
        if(status == B_OK) {
            BPath path;
            entry.GetPath(&path);
            imagePaths.Add(path.Path());
        }
    }

    /* Remove the file paths of non-images */
    for(int i = 0; i < imagePaths.CountStrings(); i++) {
        BEntry entry(imagePaths.StringAt(i));
        entry_ref ref;
        entry.GetRef(&ref);

        BMimeType type;
        BMimeType::GuessMimeType(&ref, &type);
        if(!BMimeType("image").Contains(&type))
            imagePaths.Remove(i);
    }

    if(imagePaths.IsEmpty())
        return B_NOT_INITIALIZED;

    return B_OK;
}
