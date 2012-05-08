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

#include <QDir>

#include "YaffsExporter.h"
#include "YaffsControl.h"

YaffsExporter::YaffsExporter(const QString& imgFile) {
    mImageFile = imgFile;
    mFilesExported = 0;
    mDirsExported = 0;
}

void YaffsExporter::exportItem(const YaffsItem* item, const QString& path) {
    if (item) {
        if (item->isFile()) {
            exportFile(item, path);
        } else if (item->isDir()) {
            exportDirectory(item, path);
        }
    }
}

void YaffsExporter::exportFile(const YaffsItem* item, const QString& path) {
    bool result = false;
    if (item->isFile()) {
        int headerPosition = item->getHeaderPosition();
        int filesize = item->getFileSize();
        YaffsControl yaffsControl(mImageFile.toStdString().c_str(), NULL);
        if (yaffsControl.open(YaffsControl::OPEN_READ)) {
            char* data = yaffsControl.extractFile(headerPosition);
            if (data != NULL) {
                QDir().mkpath(path);
                result = saveDataToFile(path + QDir::separator() + item->getName(), data, filesize);
                delete data;
            }
        }
    }

    if (result) {
        mFilesExported++;
    } else {
        mFileExportFailures.append(item);
    }
}

void YaffsExporter::exportDirectory(const YaffsItem* item, const QString& path) {
    bool result = false;
    if (item->isDir()) {
        result = true;
        QString dir(path);

        if (!item->isRoot()) {
            dir += QDir::separator() + item->getName();
            result = QDir().mkdir(dir);
        }

        if (result) {
            int childCount = item->childCount();
            for (int i = 0; i < childCount; ++i) {
                const YaffsItem* childItem = item->child(i);
                exportItem(childItem, dir);
            }
        }
    }

    if (result) {
        mDirsExported++;
    } else {
        mDirExportFailures.append(item);
    }
}

bool YaffsExporter::saveDataToFile(const QString& filename, const char* data, int length) {
    bool result = false;
    QFile file(filename);
    bool open = file.open(QIODevice::WriteOnly);
    if (open) {
        result = (file.write(data, length) != -1);
        file.close();
    }
    return result;
}
