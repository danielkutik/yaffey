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

#include <QDebug>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "YaffsControl.h"

unsigned char YaffsControl::mPageData[PAGE_SIZE];
unsigned char* YaffsControl::mChunkData = mPageData;
unsigned char* YaffsControl::mSpareData = mPageData + CHUNK_SIZE;

YaffsControl::YaffsControl(const char* imageFileName, YaffsControlObserver* observer) {
    mObserver = observer;

    size_t len = strlen(imageFileName);
    if (len > 0) {
        mImageFilename = new char[len + 1];
        strcpy(mImageFilename, imageFileName);
    } else {
        mImageFilename = NULL;
    }

    mImageFile = NULL;
    memset(&mSaveInfo, 0, sizeof(YaffsSaveInfo));
}

YaffsControl::~YaffsControl() {
    if (mImageFile) {
        fclose(mImageFile);
    }
    delete mImageFilename;
}

bool YaffsControl::open(OpenType openType) {
    switch (openType) {
    case OPEN_READ:
        mImageFile = fopen(mImageFilename, "rb");
        break;
    case OPEN_MODIFY:
        mImageFile = fopen(mImageFilename, "rb+");
        break;
    case OPEN_NEW:
        mImageFile = fopen(mImageFilename, "wb");
        mObjectId = YAFFS_NOBJECT_BUCKETS + 1;
        mNumPages = 0;
        break;
    }

    return (mImageFile != NULL);
}

bool YaffsControl::readImage() {
    int result = 0;
    memset(&mReadInfo, 0, sizeof(YaffsReadInfo));
    if (mImageFile) {
        while (result == 0) {
            result = readPage();
            if (result == -1) {
                if (feof(mImageFile)) {
                    mReadInfo.eofHasIncompletePage = true;
                    result = 1;
                }
                break;
            }
            processPage();
        }
    }
    mReadInfo.result = (result == 1);
    mObserver->readComplete();
    return mReadInfo.result;
}

int YaffsControl::addRoot(const yaffs_obj_hdr& objectHeader, int& headerPos) {
    headerPos = ftell(mImageFile);
    int objectId = YAFFS_OBJECTID_ROOT;
    if (!writeHeader(objectHeader, objectId)) {
        objectId = -1;
        mSaveInfo.numDirsFailed++;
    } else {
        mSaveInfo.numDirsSaved++;
    }
    return YAFFS_OBJECTID_ROOT;
}

int YaffsControl::addDirectory(const yaffs_obj_hdr& objectHeader, int& headerPos) {
    headerPos = ftell(mImageFile);
    int objectId = mObjectId++;
    if (!writeHeader(objectHeader, objectId)) {
        objectId = -1;
        mSaveInfo.numDirsFailed++;
    } else {
        mSaveInfo.numDirsSaved++;
    }
    return objectId;
}

int YaffsControl::addFile(const yaffs_obj_hdr& objectHeader, int& headerPos, const char* data, int fileSize) {
    headerPos = ftell(mImageFile);
    int objectId = mObjectId++;
    int chunks = (fileSize / CHUNK_SIZE);
    int remainder = (fileSize % CHUNK_SIZE);
    int pageGoal = chunks + (remainder > 0 ? 1 : 0);
    int pagesWritten = 0;
    bool wroteHeader = false;

    if (writeHeader(objectHeader, objectId)) {
        wroteHeader = true;
        int chunkId = 0;

        const char* dataPtr = data;
        for (int i = 0; i < chunks; ++i) {
            memcpy(mChunkData, dataPtr, CHUNK_SIZE);
            if (writePage(objectId, ++chunkId, CHUNK_SIZE)) {
                pagesWritten++;
            }
            dataPtr += CHUNK_SIZE;
        }

        if (remainder > 0) {
            memset(mChunkData + remainder, 0xff, CHUNK_SIZE - remainder);
            memcpy(mChunkData, dataPtr, remainder);
            if (writePage(objectId, ++chunkId, remainder)) {
                pagesWritten++;
            }
        }
    }

    if (wroteHeader && pagesWritten == pageGoal) {
        mSaveInfo.numFilesSaved++;
    } else {
        mSaveInfo.numFilesFailed++;
    }

    return objectId;
}

int YaffsControl::addSymLink(const yaffs_obj_hdr& objectHeader, int& headerPos) {
    headerPos = ftell(mImageFile);
    int objectId = mObjectId++;
    if (writeHeader(objectHeader, objectId)) {
        mSaveInfo.numSymLinksSaved++;
    } else {
        objectId = -1;
        mSaveInfo.numSymLinksFailed++;
    }
    return objectId;
}

bool YaffsControl::writeHeader(const yaffs_obj_hdr& objectHeader, u32 objectId) {
    bool result = false;
    if (mImageFile) {
        memset(mChunkData, 0xff, CHUNK_SIZE);
        memcpy(mChunkData, &objectHeader, sizeof(yaffs_obj_hdr));
        result = writePage(objectId, 0, 0xffff);
    }
    return result;
}

bool YaffsControl::writePage(u32 objectId, u32 chunkId, u32 numBytes) {
    bool result = false;

    static yaffs_ext_tags t;
    memset(&t, 0, sizeof(yaffs_ext_tags));
    t.chunk_used = 1;
    t.obj_id = objectId;
    t.chunk_id = chunkId;
    t.n_bytes = numBytes;
    t.serial_number = 1;
    t.seq_number = YAFFS_LOWEST_SEQUENCE_NUMBER;

    memset(mSpareData, 0xff, SPARE_SIZE);
    yaffs_packed_tags2* pt = reinterpret_cast<yaffs_packed_tags2*>(mSpareData);
    yaffs_pack_tags2(pt, &t, 1);

    if (fwrite(mPageData, PAGE_SIZE, 1, mImageFile) == 1) {
        result = true;
        mNumPages++;
    }

    return result;
}

char* YaffsControl::extractFile(int objectHeaderPos) {
    char* data = NULL;
    char* dataPtr;
    if (mImageFile) {
        if (fseek(mImageFile, objectHeaderPos, SEEK_SET) == 0) {
            if (readPage() == 0) {
                yaffs_packed_tags2* pt = (yaffs_packed_tags2*)mSpareData;
                if (pt->t.n_bytes == 0xffff) {
                    yaffs_obj_hdr* objectHeader = reinterpret_cast<yaffs_obj_hdr*>(mChunkData);
                    if (objectHeader->file_size_low > 0) {
                        data = new char[objectHeader->file_size_low];
                        dataPtr = data;
                        size_t bytesRemaining = static_cast<size_t>(objectHeader->file_size_low);
                        size_t size = 0;

                        bool success = true;
                        int readResult;
                        while (bytesRemaining > 0) {
                            readResult = readPage();
                            if (readResult == 0) {
                                size = (bytesRemaining < pt->t.n_bytes) ? bytesRemaining : pt->t.n_bytes;
                                void* dest = memcpy(dataPtr, mChunkData, size);
                                if (dest != dataPtr) {
                                    success = false;
                                    break;
                                }
                                dataPtr += size;
                            } else if (readResult == -1) {
                                success = false;
                                break;
                            }

                            bytesRemaining -= size;
                        }

                        if (!success) {
                            delete data;
                            data = NULL;
                        }
                    }
                }
            }
        }
    }

    return data;
}

bool YaffsControl::updateHeader(int objectHeaderPos, const yaffs_obj_hdr& objectHeader, int objectId) {
    bool result = false;
    if (mImageFile) {
        if (fseek(mImageFile, objectHeaderPos, SEEK_SET) == 0) {
            result = writeHeader(objectHeader, objectId);
            if (result) {
                qDebug() << "Wrote header at: " << objectHeaderPos;
            } else {
                qDebug() << "Failed to write header";
            }
        }
    }
    return result;
}

int YaffsControl::readPage() {
    int result = 0;
    memset(mPageData, 0, PAGE_SIZE);
    size_t bytesRead = fread(mPageData, 1, PAGE_SIZE, mImageFile);
    if (bytesRead != PAGE_SIZE) {
        if (bytesRead == 0) {
            result = 1;     //end of image
        } else {
            result = -1;    //error
        }
    }
    return result;
}

void YaffsControl::processPage() {
    yaffs_packed_tags2* pt = (yaffs_packed_tags2*)mSpareData;

    if (pt->t.n_bytes == 0xffff) {       //a new object
        yaffs_obj_hdr* objectHeader = reinterpret_cast<yaffs_obj_hdr*>(mChunkData);

        switch (objectHeader->type) {
            case YAFFS_OBJECT_TYPE_FILE:
                mReadInfo.numFiles++;
                break;
            case YAFFS_OBJECT_TYPE_SYMLINK:
                mReadInfo.numSymLinks++;
                break;
            case YAFFS_OBJECT_TYPE_DIRECTORY:
                mReadInfo.numDirs++;
                break;
            case YAFFS_OBJECT_TYPE_HARDLINK:
                mReadInfo.numHardLinks++;
                break;
            case YAFFS_OBJECT_TYPE_UNKNOWN:
                mReadInfo.numUnknowns++;
                break;
            case YAFFS_OBJECT_TYPE_SPECIAL:
                mReadInfo.numSpecials++;
                break;
            default:
                mReadInfo.numErrorousObjects++;
                break;
        }

        if (objectHeader->type == YAFFS_OBJECT_TYPE_FILE ||
                objectHeader->type == YAFFS_OBJECT_TYPE_DIRECTORY ||
                objectHeader->type == YAFFS_OBJECT_TYPE_SYMLINK) {

            //calculate header position to pass to observer
            long headerPos = ftell(mImageFile) - PAGE_SIZE;

            //skip over the chunks for the file data
            if (objectHeader->type == YAFFS_OBJECT_TYPE_FILE) {
                int pagePadding = PAGE_SIZE - (objectHeader->file_size_low % PAGE_SIZE);
                fseek(mImageFile, objectHeader->file_size_low + pagePadding, SEEK_CUR);
            }

            if (mObserver) {
                mObserver->newItem(pt->t.obj_id, objectHeader, headerPos);
            }
        }
    }
}
