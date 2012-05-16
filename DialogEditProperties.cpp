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

#include "DialogEditProperties.h"
#include "ui_DialogEditProperties.h"
#include "YaffsItem.h"
#include "AndroidIDs.h"

DialogEditProperties::DialogEditProperties(YaffsModel& yaffsModel, QModelIndexList& selectedRows, QWidget* parent) : QDialog(parent),
                                                                                                                     mUi(new Ui::DialogEditProperties),
                                                                                                                     mYaffsModel(yaffsModel),
                                                                                                                     mSelectedRows(selectedRows) {
    mUi->setupUi(this);

    if (mSelectedRows.size() == 1) {
        mNameIndex = mSelectedRows.at(0);
        QModelIndex parent = mNameIndex.parent();
        YaffsItem* item = static_cast<YaffsItem*>(mNameIndex.internalPointer());

        int row = mNameIndex.row();
        mPermissionsIndex = mYaffsModel.index(row, YaffsItem::PERMISSIONS, parent);
        mAliasIndex = mYaffsModel.index(row, YaffsItem::ALIAS, parent);
        mUserIndex = mYaffsModel.index(row, YaffsItem::USER, parent);
        mGroupIndex = mYaffsModel.index(row, YaffsItem::GROUP, parent);

        QString name = mNameIndex.data(Qt::EditRole).toString();
        uint permissions = mPermissionsIndex.data(Qt::EditRole).toUInt();
        QString alias = mAliasIndex.data(Qt::EditRole).toString();
        uint uid = mUserIndex.data(Qt::EditRole).toUInt();
        uint gid = mGroupIndex.data(Qt::EditRole).toUInt();

        //name & alias
        mUi->lineName->setText(name);
        if (item->isSymLink()) {
            mUi->lineAlias->setText(alias);
        } else {
            mUi->lineAlias->setEnabled(false);
        }

        //permissions
        mPermissionUpperBits = (permissions >> 12);
        mUi->chkSpecialSetUid->setChecked(permissions & SPECIAL_SETUID);
        mUi->chkSpecialSetGid->setChecked(permissions & SPECIAL_SETGID);
        mUi->chkSpecialSticky->setChecked(permissions & SPECIAL_STICKY);
        mUi->chkUserRead->setChecked(permissions & USER_READ);
        mUi->chkUserWrite->setChecked(permissions & USER_WRITE);
        mUi->chkUserExecute->setChecked(permissions & USER_EXECUTE);
        mUi->chkGroupRead->setChecked(permissions & GROUP_READ);
        mUi->chkGroupWrite->setChecked(permissions & GROUP_WRITE);
        mUi->chkGroupExecute->setChecked(permissions & GROUP_EXECUTE);
        mUi->chkAllRead->setChecked(permissions & ALL_READ);
        mUi->chkAllWrite->setChecked(permissions & ALL_WRITE);
        mUi->chkAllExecute->setChecked(permissions & ALL_EXECUTE);

        //UID & GID
        QMap<int, QString>::const_iterator i;
        mUi->boxOwnerUser->addItem("");
        mUi->boxOwnerGroup->addItem("");
        for (i = ANDROID_IDS.begin(); i != ANDROID_IDS.end(); i++) {
            const QString& idText = QString::number(i.key()) + " - " + i.value();
            const QVariant& id = i.key();
            mUi->boxOwnerUser->addItem(idText, id);
            mUi->boxOwnerGroup->addItem(idText, id);
        }

        QString uidText(QString::number(uid));
        QString gidText(QString::number(gid));
        mUi->lineOwnerUser->setText(uidText);
        mUi->lineOwnerGroup->setText(gidText);
        updateUserComboBox(uidText);
        updateGroupComboBox(gidText);
    } else {
        mUi->lineName->setEnabled(false);
        mUi->lineAlias->setEnabled(false);
        mUi->boxPermissionUser->setEnabled(false);
        mUi->boxPermissionGroup->setEnabled(false);
        mUi->boxPermissionAll->setEnabled(false);
        mUi->boxPermissionSpecial->setEnabled(false);
        mUi->boxOwnerUser->setEnabled(false);
        mUi->boxOwnerGroup->setEnabled(false);
        mUi->lineOwnerUser->setEnabled(false);
        mUi->lineOwnerGroup->setEnabled(false);
    }
}

DialogEditProperties::~DialogEditProperties() {
    delete mUi;
}

void DialogEditProperties::on_buttonBox_accepted() {
    if (mSelectedRows.size() == 1) {
        //name
        QString newName = mUi->lineName->text();
        mYaffsModel.setData(mNameIndex, newName);

        //alias
        QString newAlias = mUi->lineAlias->text();
        mYaffsModel.setData(mAliasIndex, newAlias);

        //permissions
        uint newPermissions = (mPermissionUpperBits << 12);
        newPermissions |= (mUi->chkSpecialSetUid->isChecked() ? SPECIAL_SETUID : 0);
        newPermissions |= (mUi->chkSpecialSetGid->isChecked() ? SPECIAL_SETGID : 0);
        newPermissions |= (mUi->chkSpecialSticky->isChecked() ? SPECIAL_STICKY : 0);
        newPermissions |= (mUi->chkUserRead->isChecked() ? USER_READ : 0);
        newPermissions |= (mUi->chkUserWrite->isChecked() ? USER_WRITE : 0);
        newPermissions |= (mUi->chkUserExecute->isChecked() ? USER_EXECUTE : 0);
        newPermissions |= (mUi->chkGroupRead->isChecked() ? GROUP_READ : 0);
        newPermissions |= (mUi->chkGroupWrite->isChecked() ? GROUP_WRITE : 0);
        newPermissions |= (mUi->chkGroupExecute->isChecked() ? GROUP_EXECUTE : 0);
        newPermissions |= (mUi->chkAllRead->isChecked() ? ALL_READ : 0);
        newPermissions |= (mUi->chkAllWrite->isChecked() ? ALL_WRITE : 0);
        newPermissions |= (mUi->chkAllExecute->isChecked() ? ALL_EXECUTE : 0);
        mYaffsModel.setData(mPermissionsIndex, newPermissions);

        //UID & GID
        QString uidText = mUi->lineOwnerUser->text();
        if (uidText.length() > 0) {
            uint newUid = uidText.toUInt();
            mYaffsModel.setData(mUserIndex, newUid);
        }
        QString gidText = mUi->lineOwnerGroup->text();
        if (gidText.length() > 0) {
            uint newGid = gidText.toUInt();
            mYaffsModel.setData(mGroupIndex, newGid);
        }
    }
}

void DialogEditProperties::on_boxOwnerUser_currentIndexChanged(int index) {
    QVariant uid = mUi->boxOwnerUser->itemData(index);
    mUi->lineOwnerUser->setText(uid.toString());
}

void DialogEditProperties::on_boxOwnerGroup_currentIndexChanged(int index) {
    QVariant gid = mUi->boxOwnerGroup->itemData(index);
    mUi->lineOwnerGroup->setText(gid.toString());
}

void DialogEditProperties::on_lineOwnerUser_textEdited(const QString& text) {
    QString newText(removeNonNumericChars(text));
    if (newText.length() > 1 && newText.startsWith('0')) {
        newText.remove(0, 1);
    }
    updateUserComboBox(newText);
    mUi->lineOwnerUser->setText(newText);
}

void DialogEditProperties::on_lineOwnerGroup_textEdited(const QString& text) {
    QString newText(removeNonNumericChars(text));
    updateGroupComboBox(newText);
    if (newText.length() > 1 && newText.startsWith('0')) {
        newText.remove(0, 1);
    }
    mUi->lineOwnerGroup->setText(newText);
}

QString DialogEditProperties::removeNonNumericChars(const QString& text) {
    QString newText(text);
    if (newText.length() > 0) {
        static const QRegExp regExp("[^0-9\\.]");
        newText.replace(regExp, "");
    }
    return newText;
}

void DialogEditProperties::updateUserComboBox(const QString& uidText) {
    bool gotIdMatch = false;

    if (uidText.length() > 0) {
        uint uid = uidText.toUInt();
        int count = mUi->boxOwnerUser->count();
        for (int i = 0; i < count; ++i) {
            if (mUi->boxOwnerUser->itemData(i).isValid()) {
                uint idInComboBox = mUi->boxOwnerUser->itemData(i).toUInt();
                if (uid == idInComboBox) {
                    mUi->boxOwnerUser->setCurrentIndex(i);
                    gotIdMatch = true;
                    break;
                }
            }
        }
    }

    if (!gotIdMatch) {
        mUi->boxOwnerUser->setCurrentIndex(0);
    }
}

void DialogEditProperties::updateGroupComboBox(const QString& gidText) {
    bool gotIdMatch = false;

    if (gidText.length() > 0) {
        uint gid = gidText.toUInt();
        int count = mUi->boxOwnerGroup->count();
        for (int i = 0; i < count; ++i) {
            if (mUi->boxOwnerGroup->itemData(i).isValid()) {
                uint idInComboBox = mUi->boxOwnerGroup->itemData(i).toUInt();
                if (gid == idInComboBox) {
                    mUi->boxOwnerGroup->setCurrentIndex(i);
                    gotIdMatch = true;
                    break;
                }
            }
        }
    }

    if (!gotIdMatch) {
        mUi->boxOwnerGroup->setCurrentIndex(0);
    }
}
