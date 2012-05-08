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

#ifndef DIALOGEDITPROPERTIES_H
#define DIALOGEDITPROPERTIES_H

#include <QDialog>

#include "YaffsModel.h"

namespace Ui {
    class DialogEditProperties;
}

class DialogEditProperties : public QDialog {
    Q_OBJECT

public:
    explicit DialogEditProperties(YaffsModel& yaffsModel, QModelIndexList& selectedRows, QWidget* parent = 0);
    ~DialogEditProperties();

private slots:
    void on_buttonBox_accepted();
    void on_boxOwnerUser_currentIndexChanged(int index);
    void on_boxOwnerGroup_currentIndexChanged(int index);
    void on_lineOwnerUser_textEdited(const QString& text);
    void on_lineOwnerGroup_textEdited(const QString& text);

private:
    QString removeNonNumericChars(const QString& text);
    void updateUserComboBox(const QString& uidText);
    void updateGroupComboBox(const QString& gidText);

private:
    Ui::DialogEditProperties* mUi;
    YaffsModel& mYaffsModel;
    QModelIndexList& mSelectedRows;

    QModelIndex mNameIndex;
    QModelIndex mPermissionsIndex;
    QModelIndex mAliasIndex;
    QModelIndex mUserIndex;
    QModelIndex mGroupIndex;

    uint mPermissionUpperBits;
};

#endif  //DIALOGEDITPROPERTIES_H
