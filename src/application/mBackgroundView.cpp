/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "mBackgroundView.h"
#include "../common/LockWorkstationConfig.h"
#include <string>
#include <sstream>

BBitmap* mBackgroundView::currentimage = nullptr;
bool mBackgroundView::shouldExit = false;
static uint32 multiplier;

mBackgroundView::mBackgroundView(BRect frame, const char *name, uint32 resizingMode,
    uint32 flags, rgb_color _bgcolor, uint8 bgmode, BString imgfolder, uint32 snooze)
: BView(frame, name, resizingMode, flags),
  fBackgroundColor(_bgcolor),
  fBackgroundMode(bgmode),
  fSrcPath(imgfolder),
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
    imageList.MakeEmpty(true);
    if(currentimage)
        delete currentimage;
}

void mBackgroundView::Draw(BRect updateRect)
{
    BView::Draw(updateRect);

    if(currentimage != nullptr) {
        /* From my animator mini app: resize image to bounds */
        BRect imagebounds = currentimage->Bounds().InsetByCopy(0.0f, 0.0f);
        BRect viewbounds = Bounds().InsetByCopy(0.0f, 0.0f);

        float rx = (viewbounds.Width() / imagebounds.Width());
        float ry = (viewbounds.Height() / imagebounds.Height());

        float r_ = rx <= ry ? rx : ry; // Resizing ratio

        float width = imagebounds.Width() * r_;
        float height = imagebounds.Height() * r_;

        float insert_x = (viewbounds.Width() - width) / 2.0f;
        float insert_y = (viewbounds.Height() - height) / 2.0f;

        DrawBitmap(currentimage, BRect(insert_x, insert_y,
            insert_x + width, insert_y + height));
    }

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
    BBitmap* nextimage = nullptr, *releaseimage = nullptr;

    while(!shouldExit) {
        for(int i = 0; i < localList->CountStrings(); i++) {
            nextimage = BTranslationUtils::GetBitmap(localList->StringAt(i));

            if(nextimage != NULL) {
                snooze(multiplier * 1000000);
                releaseimage = currentimage;
                currentimage = nextimage;
                delete releaseimage;
                releaseimage = nullptr;
                nextimage = nullptr;
            }

            // Dirty hack to make a linked list look like a ring
            if(i > localList->CountStrings())
                i = 0;
        }
    }
}

void mBackgroundView::InitUIData()
{
    multiplier = snoozeMultiplier;

    if(!fSrcPath.IsEmpty()) {
        switch(fBackgroundMode)
        {
            case BGM_FOLDER:
            {
                if(InitImageFolder(fSrcPath) == B_OK) {
                    fprintf(stderr, "Mode: folder\n");
                    /* Initial image */
                    currentimage = BTranslationUtils::GetBitmap(imagePaths.StringAt(0));

                    /* Image loop */
                    ThreadedCall(imageLooper, &mBackgroundView::CallSetBackgroundImage,
                        "image loop", B_DISPLAY_PRIORITY, this);
                }
                break;
            }
            case BGM_LISTFILE:
            {
                if(InitImageListFile(fSrcPath) == B_OK) {
                    fprintf(stderr, "Mode: list\n");
                    /* Initial image */
                    currentimage = BTranslationUtils::GetBitmap(imagePaths.StringAt(0));

                    /* Image loop */
                    ThreadedCall(imageLooper, &mBackgroundView::CallSetBackgroundImage,
                        "image loop", B_DISPLAY_PRIORITY, this);
                }
                break;
            }
            case BGM_STATIC:
            {
                if(InitImageStatic(fSrcPath) == B_OK) {
                    fprintf(stderr, "Mode: single\n");
                    /* Initial image */
                    currentimage = BTranslationUtils::GetBitmap(imagePaths.StringAt(0));
                }
                break;
            }
            case BGM_NONE:
                fprintf(stderr, "Mode: none\n");
            default:
                break;
        }
    }
    else
        fprintf(stderr, "Mode: no image mode\n");
}

status_t mBackgroundView::InitImageFolder(BString imageFolderPath)
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

    if(imagePaths.IsEmpty()) {
        fprintf(stderr, "Error: image folder does not contain valid images.\n");
        return B_NOT_INITIALIZED;
    }

    return B_OK;
}

status_t mBackgroundView::InitImageListFile(BString imageListPath)
{
    status_t status = B_ERROR;
    fprintf(stderr, "file string is: %s\n", imageListPath.String());

    /* Read file line by line and insert each entry into the container */
    BFile filelist(imageListPath.String(), B_READ_ONLY);
    if((status = filelist.InitCheck()) != B_OK)
        return status;

    off_t length;
    filelist.GetSize(&length);
    uint8* buffer = new uint8[length];
    filelist.Read(buffer, length);

    std::string line;
    std::istringstream iss((char*)buffer);
    while(std::getline(iss, line))
        imagePaths.Add(BString(line.c_str()));

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

    delete[] buffer;

    if(imagePaths.IsEmpty()) {
        fprintf(stderr, "Error: image list does not contain valid image paths.\n");
        return B_NOT_INITIALIZED;
    }

    return B_OK;
}

status_t mBackgroundView::InitImageStatic(BString imagePath)
{
    BEntry entry(imagePath.String());
    if(!entry.Exists())
        return B_ENTRY_NOT_FOUND;

    entry_ref ref;
    entry.GetRef(&ref);
    BMimeType type;
    BMimeType::GuessMimeType(&ref, &type);
    if(!BMimeType("image").Contains(&type)) {
        fprintf(stderr, "Error: image is not valid.\n");
        return B_BAD_DATA;
    }

    imagePaths.Add(imagePath);
    return B_OK;
}
