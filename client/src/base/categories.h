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

class Request;

class Categories : public QObject
{
    Q_OBJECT

public:
    ~Categories();

    static Categories* instance();

public Q_SLOTS:
    void add(const QString &name, const QString &path);
    CategoryList get();
    Category get(const QString &name);
    void load();
    void remove(const QString &name);

private Q_SLOTS:
    void onRequestFinished(Request *request);

Q_SIGNALS:
    void changed();
    void error(const QString &errorString);

private:
    Categories();

    static Categories* self;

    CategoryList m_categories;
};

#endif // CATEGORIES_H
