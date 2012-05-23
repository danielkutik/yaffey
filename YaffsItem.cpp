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

#include <QStringList>
#include <QDateTime>
#include <QMap>

#include "YaffsItem.h"
#include "AndroidIDs.h"

YaffsItem::YaffsItem(YaffsItem* parent, const yaffs_obj_hdr* yaffsObjectHeader, int headerPosition, int yaffsObjectId) {
    mParentItem = parent;
    if (yaffsObjectHeader != NULL) {
        memcpy(&mYaffsObjectHeader, yaffsObjectHeader, sizeof(yaffs_obj_hdr));
    }
    mHeaderPosition = headerPosition;
    mYaffsObjectId = yaffsObjectId;
    mCondition = CLEAN;
    mMarkedForDelete = false;
    mHasChildMarkedForDelete = false;
}

YaffsItem::YaffsItem(YaffsItem* parent, const QString& name, yaffs_obj_type type) {
    mParentItem = parent;

    memset(&mYaffsObjectHeader, 0xff, sizeof(yaffs_obj_hdr));
    setName(name);
    mYaffsObjectHeader.type = type;
    mYaffsObjectHeader.yst_ctime = QDateTime::currentDateTime().toTime_t();
    mYaffsObjectHeader.yst_atime = mYaffsObjectHeader.yst_ctime;
    mYaffsObjectHeader.yst_mtime = mYaffsObjectHeader.yst_ctime;

    mHeaderPosition = -1;
    mYaffsObjectId = -1;

    mCondition = NEW;
}

YaffsItem::~YaffsItem() {
    qDeleteAll(mChildItems);
}

YaffsItem* YaffsItem::createRoot() {
    YaffsItem* item = new YaffsItem(NULL, "", YAFFS_OBJECT_TYPE_DIRECTORY);

    item->mYaffsObjectId = YAFFS_OBJECTID_ROOT;
    item->mYaffsObjectHeader.parent_obj_id = item->mYaffsObjectId;
    item->mYaffsObjectHeader.yst_mode = 0771 | 0x4000;
    item->mYaffsObjectHeader.yst_uid = 0;
    item->mYaffsObjectHeader.yst_gid = 0;

    return item;
}

YaffsItem* YaffsItem::createFile(YaffsItem* parentItem, const QString& filenameWithPath, int filesize) {
    int slashPos = filenameWithPath.lastIndexOf('/');
    QString filename = filenameWithPath.mid(slashPos + 1);

    YaffsItem* item = new YaffsItem(parentItem, filename, YAFFS_OBJECT_TYPE_FILE);
    const yaffs_obj_hdr& parentHeader = parentItem->getHeader();
    item->mExternalFilename = filenameWithPath;
    item->mYaffsObjectHeader.parent_obj_id = parentItem->mYaffsObjectId;
    item->mYaffsObjectHeader.yst_mode = parentHeader.yst_mode;
    item->mYaffsObjectHeader.yst_uid = parentHeader.yst_uid;
    item->mYaffsObjectHeader.yst_gid = parentHeader.yst_gid;
    item->mYaffsObjectHeader.file_size_low = filesize;

    return item;
}

YaffsItem* YaffsItem::createDirectory(YaffsItem* parentItem, const QString& dirNameWithPath) {
    int slashPos = dirNameWithPath.lastIndexOf('/');
    QString dirName = dirNameWithPath.mid(slashPos + 1);

    YaffsItem* item = new YaffsItem(parentItem, dirName, YAFFS_OBJECT_TYPE_DIRECTORY);
    const yaffs_obj_hdr& parentHeader = parentItem->getHeader();
    item->mYaffsObjectHeader.parent_obj_id = parentItem->mYaffsObjectId;
    item->mYaffsObjectHeader.yst_mode = parentHeader.yst_mode;
    item->mYaffsObjectHeader.yst_uid = parentHeader.yst_uid;
    item->mYaffsObjectHeader.yst_gid = parentHeader.yst_gid;

    return item;
}

void YaffsItem::removeChild(int row) {
    YaffsItem* item = mChildItems.at(row);
    delete item;
    mChildItems.removeAt(row);
}

QVariant YaffsItem::data(int column) const {
    if (column == NAME) {
        return mYaffsObjectHeader.name;
    } else if (column == SIZE) {
        int fileSize = mYaffsObjectHeader.file_size_low;
        if (fileSize != -1) {
            if (fileSize >= 1048576) {
                return QString::number(fileSize / 1048576.0f, 'f', 2) + " MB";
            } else if (fileSize >= 1024) {
                return QString::number(fileSize / 1024.0f, 'f', 2) + " KB";
            } else {
                return QString::number(fileSize) + " b";
            }
        }
    } else if (column == PERMISSIONS) {
        return parseMode(mYaffsObjectHeader.yst_mode);
    } else if (column == ALIAS) {
        if (isSymLink()) {
            return mYaffsObjectHeader.alias;
        }
    } else if (column == DATE_ACCESSED) {
        QDateTime atime = QDateTime::fromTime_t(mYaffsObjectHeader.yst_atime);
        return atime.toString("dd/MM/yyyy hh:mm:ss");
    } else if (column == DATE_CREATED) {
        QDateTime ctime = QDateTime::fromTime_t(mYaffsObjectHeader.yst_ctime);
        return ctime.toString("dd/MM/yyyy hh:mm:ss");
    } else if (column == DATE_MODIFIED) {
        QDateTime mtime = QDateTime::fromTime_t(mYaffsObjectHeader.yst_mtime);
        return mtime.toString("dd/MM/yyyy hh:mm:ss");
    } else if (column == USER) {
        QString uid = ANDROID_IDS.value(mYaffsObjectHeader.yst_uid);
        if (uid.length() > 0) {
            return uid;
        } else {
            return mYaffsObjectHeader.yst_uid;
        }
    } else if (column == GROUP) {
        QString gid = ANDROID_IDS.value(mYaffsObjectHeader.yst_gid);
        if (gid.length() > 0) {
            return gid;
        } else {
            return mYaffsObjectHeader.yst_gid;
        }
    }
#ifdef QT_DEBUG
    else if (column == OBJECTID) {
        return getObjectId();
    } else if (column == PARENTID) {
        if (mParentItem) {
            return mParentItem->getObjectId();
        } else {
            return "";
        }
    } else if (column == HEADERPOS) {
        return mHeaderPosition;
    }
#endif  //QT_DEBUG
    return QVariant();
}

int YaffsItem::row() const {
    int row = 0;
    if (mParentItem) {
        row = mParentItem->mChildItems.indexOf(const_cast<YaffsItem*>(this));
    }
    return row;
}

QString YaffsItem::getFullPath() const {
    QString fullPath;
    if (isRoot()) {
        fullPath = "/";
    } else {
        const YaffsItem* item = this;
        do {
            QString name = item->getName();
            if (name.length() > 0) {
                fullPath.prepend("/" + name);
            }
        } while ((item = item->parent()));
    }
    return fullPath;
}

void YaffsItem::setName(const QString& name) {
    if (name.length() > 0) {
        QString newName = name;
        if (newName.length() > YAFFS_MAX_NAME_LENGTH) {
            newName.truncate(YAFFS_MAX_NAME_LENGTH);
        }
        size_t len = static_cast<size_t>(newName.length());
        QString currentName(mYaffsObjectHeader.name);
        if (newName != currentName) {
            memset(mYaffsObjectHeader.name, 0, YAFFS_MAX_NAME_LENGTH);
            memcpy(mYaffsObjectHeader.name, newName.toStdString().c_str(), len);
            makeDirty();
        }
    } else {
        memset(mYaffsObjectHeader.name, 0, YAFFS_MAX_NAME_LENGTH);
    }
}

void YaffsItem::setPermissions(uint permissions) {
    if (permissions != mYaffsObjectHeader.yst_mode) {
        mYaffsObjectHeader.yst_mode = permissions;
        makeDirty();
    }
}

void YaffsItem::setAlias(const QString& alias) {
    if (isSymLink()) {
        if (alias.length() > 0) {
            QString newAlias = alias;
            if (newAlias.length() > YAFFS_MAX_ALIAS_LENGTH) {
                newAlias.truncate(YAFFS_MAX_ALIAS_LENGTH);
            }
            QString currentAlias(mYaffsObjectHeader.alias);
            if (newAlias != currentAlias) {
                memset(mYaffsObjectHeader.alias, 0, YAFFS_MAX_ALIAS_LENGTH);
                strcpy(mYaffsObjectHeader.alias, newAlias.toStdString().c_str());
                makeDirty();
            }
        }
    }
}

void YaffsItem::setUserId(uint uid) {
    if (uid != mYaffsObjectHeader.yst_uid) {
        mYaffsObjectHeader.yst_uid = uid;
        makeDirty();
    }
}

void YaffsItem::setGroupId(uint gid) {
    if (gid != mYaffsObjectHeader.yst_gid) {
        mYaffsObjectHeader.yst_gid = gid;
        makeDirty();
    }
}

void YaffsItem::markForDelete() {
    //mark this item to be deleted
    mMarkedForDelete = true;

    //set all parents to having a child marked for delete
    YaffsItem* parent = mParentItem;
    while (parent) {
        if (!parent->isMarkedForDelete()) {
            parent->setHasChildMarkedForDelete(true);
        }
        parent = parent->mParentItem;
    }
}

void YaffsItem::makeDirty() {
    if (mCondition == CLEAN) {
        mCondition = DIRTY;
    }
}

QString YaffsItem::parseMode(int mode) const {
    char dest[11];

    switch (mYaffsObjectHeader.type) {
    case YAFFS_OBJECT_TYPE_FILE:
    case YAFFS_OBJECT_TYPE_HARDLINK:
        dest[0] = '-';
        break;
    case YAFFS_OBJECT_TYPE_SYMLINK:
        dest[0] = 'l';
        break;
    case YAFFS_OBJECT_TYPE_DIRECTORY:
        dest[0] = 'd';
        break;
    case YAFFS_OBJECT_TYPE_UNKNOWN:
        dest[0] = '?';
        break;
    case YAFFS_OBJECT_TYPE_SPECIAL:
        dest[0] = '!';
        break;
    }

    dest[1] = (mode & USER_READ ? 'r' : '-');
    dest[2] = (mode & USER_WRITE ? 'w' : '-');
    if (mode & SPECIAL_SETUID) {
        dest[3] = (mode & USER_EXECUTE ? 's' : 'S');
    } else {
        dest[3] = (mode & USER_EXECUTE ? 'x' : '-');
    }
    dest[4] = (mode & GROUP_READ ? 'r' : '-');
    dest[5] = (mode & GROUP_WRITE ? 'w' : '-');
    if (mode & SPECIAL_SETGID) {
        dest[6] = (mode & GROUP_EXECUTE ? 's' : 'S');
    } else {
        dest[6] = (mode & GROUP_EXECUTE ? 'x' : '-');
    }
    dest[7] = (mode & ALL_READ ? 'r' : '-');
    dest[8] = (mode & ALL_WRITE ? 'w' : '-');
    if (mode & SPECIAL_STICKY) {
        dest[9] = (mode & ALL_EXECUTE ? 't' : 'T');
    } else {
        dest[9] = (mode & ALL_EXECUTE ? 'x' : '-');
    }
    dest[10] = '\0';

    return QString(dest);
}
