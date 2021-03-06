/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "page.h"

Page::Page(QWidget *parent) :
    QWidget(parent)
{
}

QString Page::errorString() const {
    return QString();
}

Page::Status Page::status() const {
    return Idle;
}

QString Page::statusString() const {
    switch (status()) {
    case Idle:
        return tr("Idle");
    case Active:
        return tr("Active");
    case Ready:
        return tr("Ready");
    case Canceled:
        return tr("Canceled");
    case Error:
        return tr("Error");
    default:
        return QString();
    }
}
