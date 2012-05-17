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

#include "YaffsManager.h"
#include "YaffsControl.h"

YaffsManager* YaffsManager::mSelf = new YaffsManager();

YaffsManager* YaffsManager::getInstance() {
    return mSelf;
}

YaffsManager::YaffsManager() {
    mYaffsModel = NULL;
    mYaffsExportInfo = NULL;
}

YaffsManager::~YaffsManager() {
    delete mYaffsModel;
}

YaffsModel* YaffsManager::newModel() {
    delete mYaffsModel;
    mYaffsModel = new YaffsModel();
    connect(mYaffsModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(on_model_DataChanged(QModelIndex, QModelIndex)));
    connect(mYaffsModel, SIGNAL(layoutChanged()), SLOT(on_model_LayoutChanged()));
    return mYaffsModel;
}

YaffsExportInfo* YaffsManager::exportItems(QModelIndexList itemIndices, const QString& path) {
    mYaffsExportInfo = new YaffsExportInfo();
    mYaffsExportInfo->numDirsExported = 0;
    mYaffsExportInfo->numFilesExported = 0;

    foreach (QModelIndex index, itemIndices) {
        YaffsItem* item = static_cast<YaffsItem*>(index.internalPointer());
        exportItem(item, path);
    }

    return mYaffsExportInfo;
}

void YaffsManager::on_model_DataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    emit modelChanged();
}

void YaffsManager::on_model_LayoutChanged() {
    emit modelChanged();
}

void YaffsManager::exportItem(const YaffsItem* item, const QString& path) {
    if (item) {
        if (item->isFile()) {
            exportFile(item, path);
        } else if (item->isDir()) {
            exportDirectory(item, path);
        }
    }
}

void YaffsManager::exportFile(const YaffsItem* item, const QString& path) {
    bool result = false;
    if (item->isFile() && item->getCondition() != YaffsItem::NEW) {
        int headerPosition = item->getHeaderPosition();
        int filesize = item->getFileSize();
        QString imageFilename = mYaffsModel->getImageFilename();
        YaffsControl yaffsControl(imageFilename.toStdString().c_str(), NULL);
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
        mYaffsExportInfo->numFilesExported++;
    } else {
        mYaffsExportInfo->listFileExportFailures.append(item);
    }
}

void YaffsManager::exportDirectory(const YaffsItem* item, const QString& path) {
    bool result = false;
    if (item->isDir() && item->getCondition() != YaffsItem::NEW) {
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
        mYaffsExportInfo->numDirsExported++;
    } else {
        mYaffsExportInfo->listDirExportFailures.append(item);
    }
}

bool YaffsManager::saveDataToFile(const QString& filename, const char* data, int length) {
    bool result = false;
    QFile file(filename);
    bool open = file.open(QIODevice::WriteOnly);
    if (open) {
        result = (file.write(data, length) != -1);
        file.close();
    }
    return result;
}
