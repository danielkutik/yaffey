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

YaffsTreeView::YaffsTreeView(QWidget* parent) : QTreeView(parent) {
    qDebug() << "YaffsTreeView()";
}

void YaffsTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QTreeView::selectionChanged(selected, deselected);
    emit selectionChanged();
}

void YaffsTreeView::dragEnterEvent(QDragEnterEvent* event) {
    qDebug() << "dragEnterEvent";
}
/*
void YaffsTreeView::dragMoveEvent(QDragMoveEvent* event) {
    qDebug() << "dragMoveEvent";
}
*/
void YaffsTreeView::dragLeaveEvent(QDragLeaveEvent* event) {
    qDebug() << "dragLeaveEvent";
}

void YaffsTreeView::dropEvent(QDropEvent* event) {
    qDebug() << "dropEvent";
}
/*
void YaffsTreeView::extractItems() {
    if (mMimeData && !mMimeData->extracted) {
        qDebug() << "Creating files...";

        QString tmpDir = QDir::tempPath() + QDir::separator() + "YaffeyTemp" + QDir::separator();
        QString imgFile = static_cast<YaffsModel*>(model())->getImageFile();
        YaffsExtractor extractor(imgFile);

        QModelIndexList selectedRows = selectionModel()->selectedRows();
        foreach (QModelIndex index, selectedRows) {
            YaffsItem* item = static_cast<YaffsItem*>(index.internalPointer());
            tmp = new QFile(tmpDir + item->getName());
            tmp->open(QIODevice::WriteOnly);
//            extractor.extract(item, tmpDir);
        }
        mMimeData->extracted = true;
    }
}

void YaffsTreeView::actionChanged(Qt::DropAction action) {
    qDebug() << "actionChanged" << action;

//    mAction = action;
}

void YaffsTreeView::mouseReleaseEvent(QMouseEvent* event) {
    qDebug() << "mouseReleaseEvent";
    QTreeView::mouseReleaseEvent(event);

    if (mAction == Qt::MoveAction) {
        QModelIndexList selectedRows = selectionModel()->selectedRows();
        QString imgFile = static_cast<YaffsModel*>(model())->getImageFile();
        YaffsExtractor extractor(imgFile);
        foreach (QModelIndex index, selectedRows) {
            YaffsItem* item = static_cast<YaffsItem*>(index.internalPointer());
            extractor.extractFile(item, tmp);
        }
    }
}
*/
