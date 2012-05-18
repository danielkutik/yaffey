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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QMenu>

#include "YaffsModel.h"
#include "YaffsManager.h"

#define SELECTED_ROOT               0x1
#define SELECTED_DIR                0x2
#define SELECTED_FILE               0x4
#define SELECTED_SYMLINK            0x8
#define SELECTED_SINGLE             0x10

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent, QString imageFilename);
    ~MainWindow();

private slots:
    void on_treeView_doubleClicked(const QModelIndex& index);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionSaveAs_triggered();
    void on_actionImport_triggered();
    void on_actionExport_triggered();
    void on_actionExit_triggered();
    void on_actionRename_triggered();
    void on_actionDelete_triggered();
    void on_actionEditProperties_triggered();
    void on_actionAndroidFastboot_triggered();
    void on_actionAbout_triggered();
    void on_actionColumnName_triggered();
    void on_actionColumnSize_triggered();
    void on_actionColumnPermissions_triggered();
    void on_actionColumnAlias_triggered();
    void on_actionColumnDateAccessed_triggered();
    void on_actionColumnDateCreated_triggered();
    void on_actionColumnDateModified_triggered();
    void on_actionColumnUser_triggered();
    void on_actionColumnGroup_triggered();
    void on_treeViewHeader_customContextMenuRequested(const QPoint& pos);
    void on_treeView_customContextMenuRequested(const QPoint& pos);
    void on_treeView_selectionChanged();
    void on_modelChanged();

private:
    void newModel();
    void openImage(const QString& imageFilename);
    void closeImage();
    void exportSelectedItems(const QString& path);
    void setupActions();
    void updateWindowTitle();
    int identifySelection(const QModelIndexList& selectedRows);

private:
    Ui::MainWindow* mUi;                //owned
    YaffsModel* mYaffsModel;            //not owned
    YaffsManager* mYaffsManager;        //not owned - singleton
    QMenu mContextMenu;
    QMenu mHeaderContextMenu;
    QDialog* mFastbootDialog;           //owned
};

#endif  //MAINWINDOW_H
