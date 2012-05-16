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

#ifndef YAFFSTREEVIEW_H
#define YAFFSTREEVIEW_H

#include <QTreeView>
#include <QFile>

class YaffsTreeView : public QTreeView {
    Q_OBJECT

public:
    explicit YaffsTreeView(QWidget* parent = 0);

Q_SIGNALS:
    void selectionChanged();

protected:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent* event);
};

#endif  //YAFFSTREEVIEW_H
