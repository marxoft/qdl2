/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef PAGE_H
#define PAGE_H

#include <QWidget>

class Page : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)

    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Canceled,
        Error
    };

    explicit Page(QWidget *parent = 0);

    virtual QString errorString() const;

    virtual Status status() const;
    virtual QString statusString() const;

Q_SIGNALS:
    void statusChanged(Page::Status status);
};

#endif // PAGE_H
