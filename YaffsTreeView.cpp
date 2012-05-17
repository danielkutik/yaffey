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

#include <QDragEnterEvent>
#include <QUrl>
#include <QDir>

#include "YaffsTreeView.h"
#include "YaffsItem.h"
#include "YaffsModel.h"
#include "YaffsManager.h"

YaffsTreeView::YaffsTreeView(QWidget* parent) : QTreeView(parent) {
    qDebug() << "YaffsTreeView()";
}

void YaffsTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QTreeView::selectionChanged(selected, deselected);
    emit selectionChanged();
}

void YaffsTreeView::dragEnterEvent(QDragEnterEvent* event) {
    qDebug() << "dragEnterEvent";

    const QMimeData* mimeData = event->mimeData();
    if (mimeData && mimeData->hasUrls()) {
        event->accept();
    }
}

void YaffsTreeView::dragMoveEvent(QDragMoveEvent* event) {
    bool accept = false;

    QModelIndex modelIndex = indexAt(event->pos());
    if (modelIndex.isValid()) {
        YaffsItem* item = static_cast<YaffsItem*>(modelIndex.internalPointer());
        if (item) {
            if (item->isDir()) {
                accept = true;
            }
        }
    }

    if (accept) {
        event->accept();
    } else {
        event->ignore();
    }
}

void YaffsTreeView::dragLeaveEvent(QDragLeaveEvent* event) {
    qDebug() << "dragLeaveEvent";
}

void YaffsTreeView::dropEvent(QDropEvent* event) {
    qDebug() << "dropEvent";

    QModelIndex modelIndex = indexAt(event->pos());
    if (modelIndex.isValid()) {
        YaffsItem* parentItem = static_cast<YaffsItem*>(modelIndex.internalPointer());
        if (parentItem && parentItem->isDir()) {
            qDebug() << parentItem->getFullPath();

            const QMimeData* mimeData = event->mimeData();
            if (mimeData && mimeData->hasUrls()) {
                YaffsModel* yaffsManagerModel = YaffsManager::getInstance()->getModel();
                YaffsModel* yaffsModel = static_cast<YaffsModel*>(model());

                //sanity check to make sure the view is showing the model that the manager has
                if (yaffsManagerModel == yaffsModel) {
                    QList<QUrl> urls = mimeData->urls();
                    foreach (QUrl url, urls) {
                        QFileInfo fileInfo(url.toLocalFile());
                        if (fileInfo.isDir()) {
                            yaffsModel->importDirectory(parentItem, fileInfo.absoluteFilePath());
                        } else if (fileInfo.isFile()) {
                            yaffsModel->importFile(parentItem, fileInfo.absoluteFilePath());
                        }
                    }

                    event->accept();
                }
            }
        }
    }
}
