/*
 * yaffey: Utility for reading, editing and writing YAFFS2 images
 * Copyright (C) 2012 David Place <david.t.place@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef YAFFSREADER_H
#define YAFFSREADER_H

#include "Yaffs2.h"

class YaffsControlObserver {
public:
    virtual void newItem(int yaffsObjectId, const yaffs_obj_hdr* objectHeader, int fileOffset) = 0;
    virtual void readComplete() = 0;
};

struct YaffsReadInfo {
    bool result;
    bool eofHasIncompletePage;
    int numFiles;
    int numDirs;
    int numSymLinks;
    int numHardLinks;
    int numUnknowns;
    int numSpecials;
    int numErrorousObjects;
};

struct YaffsSaveInfo {
    bool result;
    int numFilesSaved;
    int numFilesFailed;
    int numDirsSaved;
    int numDirsFailed;
    int numSymLinksSaved;
    int numSymLinksFailed;
};

class YaffsControl {
public:
    enum OpenType {
        OPEN_READ,
        OPEN_MODIFY,
        OPEN_NEW
    };

    YaffsControl(const char* imageFileName, YaffsControlObserver* observer);
    ~YaffsControl();

    bool open(OpenType openType);
    bool readImage();
    YaffsReadInfo getReadInfo() { return mReadInfo; }
    YaffsSaveInfo getSaveInfo() { return mSaveInfo; }
    char* extractFile(int objectHeaderPos);
    bool updateHeader(int objectHeaderPos, const yaffs_obj_hdr& objectHeader, int objectId);

    int addRoot(const yaffs_obj_hdr& objectHeader, int& headerPos);
    int addDirectory(const yaffs_obj_hdr& objectHeader, int& headerPos);
    int addFile(const yaffs_obj_hdr& objectHeader, int& headerPos, const char* data, int fileSize);
    int addSymLink(const yaffs_obj_hdr& objectHeader, int& headerPos);

private:
    int readPage();
    void processPage();
    bool writePage(u32 objectId, u32 chunkId, u32 numBytes);
    bool writeHeader(const yaffs_obj_hdr& objectHeader, u32 objectId);

private:
    YaffsControlObserver* mObserver;
    char* mImageFilename;
    FILE* mImageFile;

    YaffsReadInfo mReadInfo;
    YaffsSaveInfo mSaveInfo;
    static u8 mPageData[];
    static u8* mChunkData;
    static u8* mSpareData;

    int mObjectId;
    int mNumPages;
};

#endif  //YAFFSREADER_H
