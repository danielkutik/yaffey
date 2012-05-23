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

#ifndef YAFFSITEM_H
#define YAFFSITEM_H

#include <QList>
#include <QVariant>
#include <QModelIndex>

#include "Yaffs2.h"

//linux permissions
#define SPECIAL_SETUID  0x800
#define SPECIAL_SETGID  0x400
#define SPECIAL_STICKY  0x200
#define USER_READ       0x100
#define USER_WRITE      0x80
#define USER_EXECUTE    0x40
#define GROUP_READ      0x20
#define GROUP_WRITE     0x10
#define GROUP_EXECUTE   0x8
#define ALL_READ        0x4
#define ALL_WRITE       0x2
#define ALL_EXECUTE     0x1

class YaffsItem {
public:
    YaffsItem(YaffsItem* parent, const yaffs_obj_hdr* yaffsObjectHeader, int headerPosition, int yaffsObjectId);
    ~YaffsItem();

    enum Condition {
        CLEAN,
        DIRTY,
        NEW,
        MOVED
    };

    enum Column {
        NAME,
        SIZE,
        PERMISSIONS,
        ALIAS,
        DATE_ACCESSED,
        DATE_CREATED,
        DATE_MODIFIED,
        USER,
        GROUP,
#ifdef QT_DEBUG
        OBJECTID,
        PARENTID,
        HEADERPOS,
#endif  //QT_DEBUG
        COLUMN_COUNT
    };

    static YaffsItem* createRoot();
    static YaffsItem* createFile(YaffsItem* parentItem, const QString& filenameWithPath, int filesize);
    static YaffsItem* createDirectory(YaffsItem* parentItem, const QString& filenameWithPath);

    QVariant data(int column) const;
    int row() const;
    QString getFullPath() const;
    void setName(const QString& name);
    void setPermissions(uint permissions);
    void setAlias(const QString& name);
    void setUserId(uint uid);
    void setGroupId(uint gid);
    void setCondition(Condition condition) { mCondition = condition; }
    void setObjectId(int objectId) { mYaffsObjectId = objectId; }
    void setParentObjectId(int parentObjectId) { mYaffsObjectHeader.parent_obj_id = parentObjectId; }
    void setHeaderPosition(int headerPos) { mHeaderPosition = headerPos; }
    void markForDelete();
    void setHasChildMarkedForDelete(bool mark) { mHasChildMarkedForDelete = mark; }
    bool isMarkedForDelete() { return mMarkedForDelete; }
    bool hasChildMarkedForDelete() { return mHasChildMarkedForDelete; }

    void appendChild(YaffsItem* child) { mChildItems.append(child); }
    void removeChild(int row);
    void clear() { mChildItems.clear(); }
    int childCount() const { return mChildItems.count(); }
    YaffsItem* parent() { return mParentItem; }
    const YaffsItem* parent() const { return mParentItem; }
    YaffsItem* child(int row) { return mChildItems.value(row); }
    const YaffsItem* child(int row) const { return mChildItems.value(row); }
    QString getName() const { return mYaffsObjectHeader.name; }
    QString getExternalFilename() const { return mExternalFilename; }
    QString getAlias() const { return mYaffsObjectHeader.alias; }
    int getHeaderPosition() const { return mHeaderPosition; }
    const yaffs_obj_hdr& getHeader() const { return mYaffsObjectHeader; }
    int getFileSize() const { return mYaffsObjectHeader.file_size_low; }
    uint getUserId() const { return mYaffsObjectHeader.yst_uid; }
    uint getGroupId() const { return mYaffsObjectHeader.yst_gid; }
    uint getPermissions() const { return mYaffsObjectHeader.yst_mode; }
    int getObjectId() const { return mYaffsObjectId; }
    bool isRoot() const { return (mParentItem == NULL); }
    bool isDir() const { return mYaffsObjectHeader.type == YAFFS_OBJECT_TYPE_DIRECTORY; }
    bool isFile() const { return mYaffsObjectHeader.type == YAFFS_OBJECT_TYPE_FILE; }
    bool isSymLink() const { return mYaffsObjectHeader.type == YAFFS_OBJECT_TYPE_SYMLINK; }
    Condition getCondition() const { return mCondition; }

private:
    YaffsItem(YaffsItem* parent, const QString& name, yaffs_obj_type type);
    QString parseMode(int mode) const;
    void makeDirty();

private:
    YaffsItem* mParentItem;
    int mHeaderPosition;
    int mYaffsObjectId;
    QList<YaffsItem*> mChildItems;
    yaffs_obj_hdr mYaffsObjectHeader;
    Condition mCondition;
    QString mExternalFilename;      //filename with path - only for new files
    bool mMarkedForDelete;
    bool mHasChildMarkedForDelete;
};

#endif  //YAFFSITEM_H
