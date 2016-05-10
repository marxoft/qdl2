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

#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <QObject>
#include <QList>

struct Category
{
    Category() :
        name(QString()),
        path(QString())
    {
    }
    
    Category(const QString &n, const QString &p) :
        name(n),
        path(p)
    {
    }
    
    QString name;
    QString path;
};

typedef QList<Category> CategoryList;

class Categories : public QObject
{
    Q_OBJECT

public:
    ~Categories();

    static Categories* instance();

public Q_SLOTS:
    static bool add(const QString &name, const QString &path);
    static CategoryList get();
    static Category get(const QString &name);
    static bool remove(const QString &name);

Q_SIGNALS:
    void changed();

private:
    Categories();

    static Categories* self;
};

#endif // CATEGORIES_H
