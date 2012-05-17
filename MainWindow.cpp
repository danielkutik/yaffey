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

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QListView>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DialogEditProperties.h"
#include "DialogFastboot.h"
#include "DialogImport.h"
#include "YaffsManager.h"
#include "YaffsTreeView.h"

static const QString APPNAME = "Yaffey";
static const QString VERSION = "0.1";

MainWindow::MainWindow(QWidget* parent, QString imageFilename) : QMainWindow(parent),
                                                                 mUi(new Ui::MainWindow),
                                                                 mContextMenu(this) {
    mUi->setupUi(this);

    //setup context menu for the treeview
    mContextMenu.addAction(mUi->actionImport);
    mContextMenu.addAction(mUi->actionExport);
    mContextMenu.addSeparator();
    mContextMenu.addAction(mUi->actionRename);
    mContextMenu.addAction(mUi->actionDelete);
    mContextMenu.addSeparator();
    mContextMenu.addAction(mUi->actionEditProperties);

    //setup context menu for the header
    mHeaderContextMenu.addAction(mUi->actionColumnName);
    mHeaderContextMenu.addAction(mUi->actionColumnSize);
    mHeaderContextMenu.addAction(mUi->actionColumnPermissions);
    mHeaderContextMenu.addAction(mUi->actionColumnAlias);
    mHeaderContextMenu.addAction(mUi->actionColumnDateAccessed);
    mHeaderContextMenu.addAction(mUi->actionColumnDateCreated);
    mHeaderContextMenu.addAction(mUi->actionColumnDateModified);
    mHeaderContextMenu.addAction(mUi->actionColumnUser);
    mHeaderContextMenu.addAction(mUi->actionColumnGroup);

    //get YaffsManager instance and create model
    mYaffsManager = YaffsManager::getInstance();
    newModel();
    updateWindowTitle();

    QHeaderView* headerView = mUi->treeView->header();
    headerView->setContextMenuPolicy(Qt::CustomContextMenu);
    headerView->setResizeMode(YaffsItem::NAME, QHeaderView::Stretch);
    headerView->setResizeMode(YaffsItem::SIZE, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::PERMISSIONS, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::ALIAS, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::DATE_ACCESSED, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::DATE_CREATED, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::DATE_MODIFIED, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::USER, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::GROUP, QHeaderView::ResizeToContents);
#ifdef QT_DEBUG
    headerView->setResizeMode(YaffsItem::OBJECTID, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::PARENTID, QHeaderView::ResizeToContents);
    headerView->setResizeMode(YaffsItem::HEADERPOS, QHeaderView::ResizeToContents);
#endif  //QT_DEBUG

    mUi->treeView->hideColumn(YaffsItem::DATE_CREATED);
    mUi->treeView->hideColumn(YaffsItem::DATE_ACCESSED);

    mUi->actionColumnName->setEnabled(false);
    mUi->actionColumnName->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::NAME));
    mUi->actionColumnSize->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::SIZE));
    mUi->actionColumnPermissions->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::PERMISSIONS));
    mUi->actionColumnAlias->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::ALIAS));
    mUi->actionColumnDateAccessed->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::DATE_ACCESSED));
    mUi->actionColumnDateCreated->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::DATE_CREATED));
    mUi->actionColumnDateModified->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::DATE_MODIFIED));
    mUi->actionColumnUser->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::USER));
    mUi->actionColumnGroup->setChecked(!mUi->treeView->isColumnHidden(YaffsItem::GROUP));

    connect(headerView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(on_treeViewHeader_customContextMenuRequested(QPoint)));
    connect(mUi->actionExpandAll, SIGNAL(triggered()), mUi->treeView, SLOT(expandAll()));
    connect(mUi->actionCollapseAll, SIGNAL(triggered()), mUi->treeView, SLOT(collapseAll()));
    connect(mUi->treeView, SIGNAL(selectionChanged()), SLOT(on_treeView_selectionChanged()));

    if (imageFilename.length() > 0) {
        show();
        openImage(imageFilename);
    } else {
        mUi->statusBar->showMessage(windowTitle() + " v" + VERSION);
    }

    mFastbootDialog = NULL;

    setupActions();
}

MainWindow::~MainWindow() {
    delete mUi;
    delete mFastbootDialog;
}

void MainWindow::newModel() {
    mYaffsModel = mYaffsManager->newModel();
    mUi->treeView->setModel(mYaffsModel);
    connect(mYaffsManager, SIGNAL(modelChanged()), SLOT(on_modelChanged()));
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex& itemIndex) {
    YaffsItem* item = static_cast<YaffsItem*>(itemIndex.internalPointer());
    if (item) {
        if (item->isFile() || item->isSymLink()) {
            mUi->actionEditProperties->trigger();
        }
    }
}

void MainWindow::on_actionNew_triggered() {
    newModel();
    mYaffsModel->newImage("new-yaffs2.img");
    mUi->statusBar->showMessage("Created new YAFFS2 image");
}

void MainWindow::on_actionOpen_triggered() {
    QString imageFilename = QFileDialog::getOpenFileName(this, "Open File", ".");

    if (imageFilename.length() > 0) {
        newModel();
        openImage(imageFilename);
    }
}

void MainWindow::updateWindowTitle() {
    QString modelFilename = mYaffsModel->getImageFilename();
    if (modelFilename.length() > 0) {
        if (mYaffsModel->isDirty()) {
            setWindowTitle(APPNAME + " - " + modelFilename + "*");
        } else {
            setWindowTitle(APPNAME + " - " + modelFilename);
        }
    } else {
        setWindowTitle(APPNAME);
    }
}

void MainWindow::openImage(const QString& imageFilename) {
    if (imageFilename.length() > 0) {
        YaffsReadInfo readInfo = mYaffsModel->openImage(imageFilename);
        if (readInfo.result) {
            QModelIndex rootIndex = mYaffsModel->index(0, 0);
            mUi->treeView->expand(rootIndex);
            mUi->statusBar->showMessage("Opened image: " + imageFilename);

            updateWindowTitle();
            QString summary("<table>" \
                            "<tr><td width=120>Files:</td><td>" + QString::number(readInfo.numFiles) + "</td></tr>" +
                            "<tr><td width=120>Directories:</td><td>" + QString::number(readInfo.numDirs) + "</td></tr>" +
                            "<tr><td width=120>SymLinks:</td><td>" + QString::number(readInfo.numSymLinks) + "</td></tr>" +
                            "<tr><td colspan=2><hr/></td></tr>" +
                            "<tr><td width=120>HardLinks:</td><td>" + QString::number(readInfo.numHardLinks) + "</td></tr>" +
                            "<tr><td width=120>Specials:</td><td>" + QString::number(readInfo.numSpecials) + "</td></tr>" +
                            "<tr><td width=120>Unknowns:</td><td>" + QString::number(readInfo.numUnknowns) + "</td></tr>" +
                            "<tr><td colspan=2><hr/></td></tr>" +
                            "<tr><td width=120>Errors:</td><td>" + QString::number(readInfo.numErrorousObjects) + "</td></tr></table>");

            if (readInfo.eofHasIncompletePage) {
                summary += "<br/><br/>Warning:<br/>Incomplete page found at end of file";
            }
            QMessageBox::information(this, "Summary", summary);
        } else {
            QString msg = "Error opening image: " + imageFilename;
            mUi->statusBar->showMessage(msg);
            QMessageBox::critical(this, "Error", msg);
        }
        setupActions();
    }
}

void MainWindow::on_actionClose_triggered() {
    QString imageFile = mYaffsModel->getImageFilename();
    if (imageFile.length() > 0) {
        newModel();
        mUi->statusBar->showMessage("Closed image file: " + imageFile);
    }
    mUi->linePath->clear();
    updateWindowTitle();
    setupActions();
}

void MainWindow::on_actionSave_triggered() {
/*    if (mYaffsModel->isImageOpen()) {
        QString imageFile = mYaffsModel->getImageFilename();
        bool saved = mYaffsModel->save();
        if (saved) {
            mUi->statusBar->showMessage("Image saved: " + imageFile);
        } else {
            QString msg = "Error saving image: " + imageFile;
            QMessageBox::critical(this, "Error", msg);
            mUi->statusBar->showMessage(msg);
        }
    } else {
        mUi->statusBar->showMessage("Nothing to save");
    }*/
}

void MainWindow::on_actionSaveAs_triggered() {
    if (mYaffsModel->isImageOpen()) {
        QString imgName = mYaffsModel->getImageFilename();
        QString saveAsFilename = QFileDialog::getSaveFileName(this, "Save Image As", "./" + imgName);
        if (saveAsFilename.length() > 0) {
            YaffsSaveInfo saveInfo = mYaffsModel->saveAs(saveAsFilename);
            updateWindowTitle();
            if (saveInfo.result) {
                mUi->statusBar->showMessage("Image saved: " + saveAsFilename);
                QString summary("<table>" \
                                "<tr><td width=120>Files:</td><td>" + QString::number(saveInfo.numFilesSaved) + "</td></tr>" +
                                "<tr><td width=120>Directories:</td><td>" + QString::number(saveInfo.numDirsSaved) + "</td></tr>" +
                                "<tr><td width=120>SymLinks:</td><td>" + QString::number(saveInfo.numSymLinksSaved) + "</td></tr>" +
                                "<tr><td colspan=2><hr/></td></tr>" +
                                "<tr><td width=120>Files Failed:</td><td>" + QString::number(saveInfo.numFilesFailed) + "</td></tr>" +
                                "<tr><td width=120>Directories Failed:</td><td>" + QString::number(saveInfo.numDirsFailed) + "</td></tr>" +
                                "<tr><td width=120>SymLinks Failed:</td><td>" + QString::number(saveInfo.numSymLinksFailed) + "</td></tr></td></tr></table>");
                QMessageBox::information(this, "Save summary", summary);
            } else {
                QString msg = "Error saving image: " + saveAsFilename;
                QMessageBox::critical(this, "Error", msg);
                mUi->statusBar->showMessage(msg);
            }
        }
    }
}

void MainWindow::on_actionImport_triggered() {
    DialogImport import(this);
    int result = import.exec();

    if (result == DialogImport::RESULT_FILE) {
        QModelIndex parentIndex = mUi->treeView->selectionModel()->currentIndex();
        YaffsItem* parentItem = static_cast<YaffsItem*>(parentIndex.internalPointer());
        if (parentItem && parentItem->isDir()) {
            QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select file(s) to import...");
            foreach (QString importFilename, fileNames) {
                importFilename.replace('\\', '/');
                mYaffsModel->importFile(parentItem, importFilename);
            }
        }
    } else if (result == DialogImport::RESULT_DIRECTORY) {
        QModelIndex parentIndex = mUi->treeView->selectionModel()->currentIndex();
        YaffsItem* parentItem = static_cast<YaffsItem*>(parentIndex.internalPointer());
        if (parentItem && parentItem->isDir()) {
            QString directoryName = QFileDialog::getExistingDirectory(this, "Select directory to import...");
            if (directoryName.length() > 0) {
                directoryName.replace('\\', '/');
                mYaffsModel->importDirectory(parentItem, directoryName);
            }
        }
    }
}

void MainWindow::on_actionExport_triggered() {
    QModelIndexList selectedRows = mUi->treeView->selectionModel()->selectedRows();
    if (selectedRows.size() > 0) {
        QString path = QFileDialog::getExistingDirectory(this);
        if (path.length() > 0) {
            exportSelectedItems(path);
        } else {
            mUi->statusBar->showMessage("Export cancelled");
        }
    } else {
        mUi->statusBar->showMessage("Nothing selected to export");
    }
}

void MainWindow::on_actionExit_triggered() {
    close();
}

void MainWindow::on_actionRename_triggered() {
    QModelIndex index = mUi->treeView->selectionModel()->currentIndex();
    YaffsItem* item = static_cast<YaffsItem*>(index.internalPointer());
    if (item && !item->isRoot()) {
        if (index.column() == YaffsItem::NAME) {
            mUi->treeView->edit(index);
        }
    }
}

void MainWindow::on_actionDelete_triggered() {
    QModelIndexList selectedRows = mUi->treeView->selectionModel()->selectedRows();
    int selectedRowCount = selectedRows.size();
    if (selectedRowCount == 1) {
/*        QModelIndex itemIndex = selectedRows.at(0);
        mUi->treeView->selectionModel()->clearSelection();
        QVariant name = itemIndex.data();
        if (mYaffsModel->removeRow(itemIndex.row(), itemIndex.parent())) {
            mUi->statusBar->showMessage("Deleted item: " + name.toString());
        }*/

        int deleted = 0;
        for (int i = selectedRows.size() - 1; i >= 0; --i) {
            QModelIndex index = selectedRows.at(i);
            mYaffsModel->removeRow(index.row(), index.parent());
            deleted++;
        }
        mUi->statusBar->showMessage("Deleted " + QString::number(deleted) + " items");
    } else if (selectedRowCount > 1) {
        mUi->statusBar->showMessage("Currently only deleting 1 object at a time is supported");
    }
}

void MainWindow::on_actionEditProperties_triggered() {
    QModelIndexList selectedRows = mUi->treeView->selectionModel()->selectedRows();
    if (selectedRows.size() > 0) {
        QDialog* dialog = new DialogEditProperties(*mYaffsModel, selectedRows, this);
        dialog->exec();
    }
    setupActions();
}

void MainWindow::on_actionAndroidFastboot_triggered() {
    if (mFastbootDialog) {
        mFastbootDialog->show();
    } else {
        mFastbootDialog = new DialogFastboot(this);
        mFastbootDialog->exec();
    }
}

void MainWindow::on_actionAbout_triggered() {
    static const QString about("<b>" + APPNAME + " v" + VERSION + "</b><br/>" \
                               "Yet Another Flash File (System) Editor YEAH!<br/><br/>" \
                               "Built on " + QString(__DATE__) + " at " + QString(__TIME__) + "<br/><br/>" \
                               "Written by David Place<br/><br/>" \
                               "Special thanks to Dan Lawrence");
    QMessageBox::information(this, "About " + APPNAME, about);
}

void MainWindow::on_actionColumnName_triggered() {
    if (mUi->actionColumnName->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::NAME);
    } else {
        mUi->treeView->hideColumn(YaffsItem::NAME);
    }
}

void MainWindow::on_actionColumnSize_triggered() {
    if (mUi->actionColumnSize->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::SIZE);
    } else {
        mUi->treeView->hideColumn(YaffsItem::SIZE);
    }
}

void MainWindow::on_actionColumnPermissions_triggered() {
    if (mUi->actionColumnPermissions->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::PERMISSIONS);
    } else {
        mUi->treeView->hideColumn(YaffsItem::PERMISSIONS);
    }
}

void MainWindow::on_actionColumnAlias_triggered() {
    if (mUi->actionColumnAlias->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::ALIAS);
    } else {
        mUi->treeView->hideColumn(YaffsItem::ALIAS);
    }
}

void MainWindow::on_actionColumnDateAccessed_triggered() {
    if (mUi->actionColumnDateAccessed->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::DATE_ACCESSED);
    } else {
        mUi->treeView->hideColumn(YaffsItem::DATE_ACCESSED);
    }
}

void MainWindow::on_actionColumnDateCreated_triggered() {
    if (mUi->actionColumnDateCreated->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::DATE_CREATED);
    } else {
        mUi->treeView->hideColumn(YaffsItem::DATE_CREATED);
    }
}

void MainWindow::on_actionColumnDateModified_triggered() {
    if (mUi->actionColumnDateModified->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::DATE_MODIFIED);
    } else {
        mUi->treeView->hideColumn(YaffsItem::DATE_MODIFIED);
    }
}

void MainWindow::on_actionColumnUser_triggered() {
    if (mUi->actionColumnUser->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::USER);
    } else {
        mUi->treeView->hideColumn(YaffsItem::USER);
    }
}

void MainWindow::on_actionColumnGroup_triggered() {
    if (mUi->actionColumnGroup->isChecked()) {
        mUi->treeView->showColumn(YaffsItem::GROUP);
    } else {
        mUi->treeView->hideColumn(YaffsItem::GROUP);
    }
}

void MainWindow::on_treeViewHeader_customContextMenuRequested(const QPoint& pos) {
    QPoint p(mUi->treeView->mapToGlobal(pos));
    mHeaderContextMenu.exec(p);
}

void MainWindow::on_treeView_customContextMenuRequested(const QPoint& pos) {
    setupActions();

    QPoint p(mUi->treeView->mapToGlobal(pos));
    p.setY(p.y() + mUi->treeView->header()->height());
    mContextMenu.exec(p);
}

void MainWindow::on_modelChanged() {
    setupActions();
}

void MainWindow::on_treeView_selectionChanged() {
    setupActions();
}

void MainWindow::exportSelectedItems(const QString& path) {
    QModelIndexList selectedRows = mUi->treeView->selectionModel()->selectedRows();
    if (selectedRows.size() > 0) {
        foreach (QModelIndex index, selectedRows) {
            YaffsItem* item = static_cast<YaffsItem*>(index.internalPointer());
            mYaffsManager->exportItem(item, path);
        }

        QString status = "Exported " + QString::number(mYaffsManager->getDirExportCount()) + " dir(s) and " +
                                       QString::number(mYaffsManager->getFileExportCount()) + " file(s).";
        mUi->statusBar->showMessage(status);

        int dirFails = mYaffsManager->getDirExportFailures().size();
        int fileFails = mYaffsManager->getFileExportFailures().size();
        if (dirFails + fileFails > 0) {
            QString msg;

            if (dirFails > 0) {
                static const int MAXDIRS = 10;
                QString items;
                int max = (dirFails > MAXDIRS ? MAXDIRS : dirFails);
                const QList<const YaffsItem*> list = mYaffsManager->getDirExportFailures();
                for (int i = 0; i < max; ++i) {
                    const YaffsItem* item = list.at(i);
                    items += item->getFullPath() + "\n";
                }
                msg += "Failed to export directories:\n" + items;

                if (dirFails > MAXDIRS) {
                    msg += "... plus " + QString::number(dirFails - MAXDIRS) + " more";
                }
            }

            if (fileFails > 0) {
                if (dirFails > 0) {
                    msg += "\n";
                }

                static const int MAXFILES = 10;
                QString items;
                int max = (fileFails > MAXFILES ? MAXFILES : dirFails);
                const QList<const YaffsItem*> list = mYaffsManager->getFileExportFailures();
                for (int i = 0; i < max; ++i) {
                    const YaffsItem* item = list.at(i);
                    items += item->getFullPath() + "\n";
                }
                msg += "Failed to export files:\n" + items;

                if (fileFails > MAXFILES) {
                    msg += "... plus " + QString::number(fileFails - MAXFILES) + " more";
                }
            }

            QMessageBox::critical(this, "Export", msg);
        }
    }
}

int MainWindow::identifySelection(const QModelIndexList& selectedRows) {
    int selectionFlags = (selectedRows.size() == 1 ? SELECTED_SINGLE : 0);

    //iterate through the list of items to see what we have selected
    YaffsItem* item = NULL;
    foreach (QModelIndex index, selectedRows) {
        item = static_cast<YaffsItem*>(index.internalPointer());
        if (item) {
            selectionFlags |= (item->isRoot() ? SELECTED_ROOT : 0);
            selectionFlags |= (item->isDir() ? SELECTED_DIR : 0);
            selectionFlags |= (item->isFile() ? SELECTED_FILE : 0);
            selectionFlags |= (item->isSymLink() ? SELECTED_SYMLINK : 0);
        }
    }

    return selectionFlags;
}

void MainWindow::setupActions() {
    updateWindowTitle();

    QModelIndexList selectedRows = mUi->treeView->selectionModel()->selectedRows();
    int selectionFlags = identifySelection(selectedRows);
    int selectionSize = selectedRows.size();

    if (mYaffsModel->index(0, 0).isValid()) {
        mUi->actionExpandAll->setEnabled(true);
        mUi->actionCollapseAll->setEnabled(true);
        mUi->actionSaveAs->setEnabled(true);
    } else {
        mUi->actionExpandAll->setEnabled(false);
        mUi->actionCollapseAll->setEnabled(false);
        mUi->actionSaveAs->setEnabled(false);
    }

    if (mYaffsModel->isDirty()) {
        mUi->actionSave->setEnabled(true);
    } else {
        mUi->actionSave->setEnabled(false);
    }

    mUi->actionEditProperties->setEnabled(false);
    mUi->actionImport->setEnabled(false);
    mUi->actionExport->setEnabled(false);
    mUi->actionRename->setEnabled(false);
    mUi->actionDelete->setEnabled(false);

    //if only a single item is selected
    if (selectionSize == 1) {
        mUi->actionRename->setEnabled(!(selectionFlags & SELECTED_ROOT));
        mUi->actionImport->setEnabled(  selectionFlags & SELECTED_DIR);

        QModelIndex itemIndex = selectedRows.at(0);
        YaffsItem* item = static_cast<YaffsItem*>(itemIndex.internalPointer());
        if (item) {
            mUi->linePath->setText(item->getFullPath());
        }
    }

    if (selectionSize >= 1) {
        mUi->actionDelete->setEnabled(!(selectionFlags & SELECTED_ROOT));
        mUi->actionEditProperties->setEnabled(!(selectionFlags & SELECTED_ROOT));
        mUi->actionExport->setEnabled((selectionFlags & (SELECTED_DIR | SELECTED_FILE) && !(selectionFlags & SELECTED_SYMLINK)));

        mUi->statusBar->showMessage("Selected " + QString::number(selectedRows.size()) + " items");
    } else if (selectionSize == 0) {
        mUi->statusBar->showMessage("");
    }
}
