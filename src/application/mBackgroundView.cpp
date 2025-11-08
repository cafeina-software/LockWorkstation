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
    uint32 flags, const LWSettings* settings)
: BView(frame, name, resizingMode, flags),
  fCurrentSettings(settings),
  fBackgroundColor(settings->BackgroundColor()),
  fBackgroundMode(settings->BackgroundMode()),
  snoozeMultiplier(settings->BackgroundImageSnooze()),
  fBackgroundImageAdjust(settings->BackgroundImageAdjustment()),
  fSrcPath("")
{
    SetDrawingMode(B_OP_OVER);
    SetViewColor(fBackgroundColor);

    InitUIData();
}

mBackgroundView::~mBackgroundView()
{
    shouldExit = true;
    //kill_thread(imageLooper);

    if(currentimage)
        delete currentimage;
}

// #pragma mark - Public methods

void mBackgroundView::Draw(BRect updateRect)
{
    BView::Draw(updateRect);

    if(currentimage != nullptr) {
        BRect imagebounds = currentimage->Bounds().InsetByCopy(0.0f, 0.0f);
        BRect viewbounds = Bounds().InsetByCopy(0.0f, 0.0f);
        BRect resultrect;

        switch(fBackgroundImageAdjust) {
            case BGI_ADJ_KEEP_AND_CENTER:
                keep_center_view(viewbounds, imagebounds, &resultrect);
                break;
            case BGI_ADJ_STRETCH_TO_SCREEN:
                fullscreen_view(viewbounds, &resultrect);
                break;
            case BGI_ADJ_SCALE_X:
                full_x_view(viewbounds, imagebounds, &resultrect);
                break;
            case BGI_ADJ_SCALE_Y:
                full_y_view(viewbounds, imagebounds, &resultrect);
                break;
            default:
            case BGI_ADJ_SCALE_X_Y:
                proportional_view(viewbounds, imagebounds, &resultrect);
                break;
        }

        DrawBitmap(currentimage, resultrect);
    }

    Invalidate();
}

// #pragma mark - Init

void mBackgroundView::InitUIData()
{
    multiplier = snoozeMultiplier;

    switch(fBackgroundMode)
    {
        case BGM_FOLDER:
        {
            fSrcPath.SetTo(fCurrentSettings->BackgroundImageFolderPath());

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
            fSrcPath.SetTo(fCurrentSettings->BackgroundImageListPath());

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
            fSrcPath.SetTo(fCurrentSettings->BackgroundImageStaticPath());

            if(InitImageStatic(fSrcPath) == B_OK) {
                fprintf(stderr, "Mode: single\n");
                /* Initial image */
                currentimage = BTranslationUtils::GetBitmap(imagePaths.StringAt(0));
            }
            break;
        }
        case BGM_NONE:
        default:
            fprintf(stderr, "Mode: none\n");
            break;
    }
}

status_t mBackgroundView::InitImageStatic(BString imagePath)
{
    if(imagePath.IsEmpty())
        return B_BAD_VALUE;

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

status_t mBackgroundView::InitImageFolder(BString imageFolderPath)
{
    if(imageFolderPath.IsEmpty())
        return B_BAD_VALUE;

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
    for(int i = imagePaths.CountStrings() - 1; i >= 0; i--) {
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
    if(imageListPath.IsEmpty())
        return B_BAD_VALUE;

    status_t status = B_ERROR;
    fprintf(stderr, "file string is: %s\n", imageListPath.String());

    /* Read file line by line and insert each entry into the container */
    BFile filelist(imageListPath.String(), B_READ_ONLY);
    if((status = filelist.InitCheck()) != B_OK)
        return status;

    off_t length;
    filelist.GetSize(&length);
    char* buffer = new char[length + 1];
    filelist.Read(buffer, length);
    buffer[length] = '\0';

    std::string line;
    std::istringstream iss(buffer);
    while(std::getline(iss, line))
        imagePaths.Add(BString(line.c_str()));

    /* Remove the file paths of non-images */
    for(int i = imagePaths.CountStrings() - 1; i >= 0; i--) {
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

// #pragma mark - Image display

int32 mBackgroundView::CallSetBackgroundImage(void* data)
{
    mBackgroundView* view = (mBackgroundView*) data;
    view->SetBackgroundImage(&view->imagePaths);
    return 0;
}

void mBackgroundView::SetBackgroundImage(void* data)
{
    BStringList* localList = (BStringList*)data;

    while(!shouldExit) {
        BBitmap* nextimage = nullptr, *releaseimage = nullptr;
        for(int i = 0; i < localList->CountStrings(); i++) {
            nextimage = BTranslationUtils::GetBitmap(localList->StringAt(i).String());

            if(nextimage != NULL) {
                snooze(multiplier * 1000000);
                releaseimage = currentimage;
                currentimage = nextimage;
                if(releaseimage)
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

// #pragma mark - Image adjustment

void mBackgroundView::keep_center_view(BRect viewRect, BRect imageRect, BRect* resultRect)
{
    float rx = viewRect.Width() / imageRect.Width();
    float ry = viewRect.Height() / imageRect.Height();

    float sx = rx < 1 ? viewRect.left - (imageRect.Width() - viewRect.Width()) / 2.0f
                      : viewRect.left + (viewRect.Width() - imageRect.Width()) / 2.0f;
    float sy = ry < 1 ? viewRect.top - (imageRect.Height() - viewRect.Height()) / 2.0f
                      : viewRect.top + (viewRect.Height() - imageRect.Height()) / 2.0f;

    float ex = sx + imageRect.Width();
    float ey = sy + imageRect.Height();

    resultRect->Set(sx, sy, ex, ey);
}

void mBackgroundView::full_x_view(BRect viewRect, BRect imageRect, BRect* resultRect)
{
    float rx = viewRect.Width() / imageRect.Width();
    float ry = viewRect.Height() / imageRect.Height();

    float newh = imageRect.Height() * rx;

    float ey = ry < 1 ? viewRect.Height() + (newh - viewRect.Height()) / 2.0f
                     : viewRect.Height() - (viewRect.Height() - newh) / 2.0f;

    float sy = ry < 1 ? viewRect.top - (newh - viewRect.Height()) / 2.0f
                      : viewRect.top + (viewRect.Height() - newh) / 2.0f;

    resultRect->Set(viewRect.left, sy, viewRect.Width(), ey);
}

void mBackgroundView::full_y_view(BRect viewRect, BRect imageRect, BRect* resultRect)
{
    float rx = viewRect.Width() / imageRect.Width();
    float ry = viewRect.Height() / imageRect.Height();

    float neww = imageRect.Width() * ry;

    float ex = rx < 1 ? viewRect.Width() + (neww - viewRect.Width()) / 2.0f
                     : viewRect.Width() - (viewRect.Width() - neww) / 2.0f;

    float sx = rx < 1 ? viewRect.left - (neww - viewRect.Width()) / 2.0f
                      : viewRect.left + (viewRect.Width() - neww) / 2.0f;

    resultRect->Set(sx, viewRect.top, ex, viewRect.Height());
}

void mBackgroundView::proportional_view(BRect viewRect, BRect imageRect, BRect* resultRect)
{
    /* From my animator mini app: resize image to bounds */
    float rx = (viewRect.Width() / imageRect.Width());
    float ry = (viewRect.Height() / imageRect.Height());
    float rfactor = rx <= ry ? rx : ry; // Resizing ratio

    float w = imageRect.Width() * rfactor;
    float h = imageRect.Height() * rfactor;
    float sx = (viewRect.Width() - w) / 2.0f;
    float sy = (viewRect.Height() - h) / 2.0f;

    resultRect->Set(sx, sy, sx + w, sy + h);
}

void mBackgroundView::fullscreen_view(BRect viewRect, BRect* resultRect)
{
    resultRect->Set(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom);
}
