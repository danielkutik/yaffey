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

#ifndef YAFFSMANAGER_H
#define YAFFSMANAGER_H

#include <QList>
#include <QFile>

#include "YaffsItem.h"

class YaffsManager {
public:
    YaffsManager(const QString& imgFile);
    void exportItem(const YaffsItem* item, const QString& path);
    int getFileExportCount() const { return mFilesExported; }
    int getDirExportCount() const { return mDirsExported; }
    const QList<const YaffsItem*>& getFileExportFailures() const { return mFileExportFailures; }
    const QList<const YaffsItem*>& getDirExportFailures() const { return mDirExportFailures; }

private:
    void exportFile(const YaffsItem* item, const QString& path);
    void exportDirectory(const YaffsItem* item, const QString& path);
    bool saveDataToFile(const QString& filename, const char* data, int length);

private:
    QString mImageFile;
    QList<const YaffsItem*> mFileExportFailures;
    QList<const YaffsItem*> mDirExportFailures;
    int mFilesExported;
    int mDirsExported;
};

#endif  //YAFFSMANAGER_H
