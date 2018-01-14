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

#include "categories.h"
#include "request.h"

Categories* Categories::self;

Categories::Categories() :
    QObject()
{
}

Categories::~Categories() {
    self = 0;
}

Categories* Categories::instance() {
    return self ? self : self = new Categories;
}

void Categories::add(const QString &name, const QString &path) {
    QVariantMap category;
    category["name"] = name;
    category["path"] = path;
    Request *request = new Request(this);
    request->post("/categories/addCategory", category);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

CategoryList Categories::get() {
    return m_categories;
}

Category Categories::get(const QString &name) {
    foreach (const Category &category, m_categories) {
        if (category.name == name) {
            return category;
        }
    }

    return Category();
}

void Categories::load() {
    Request *request = new Request(this);
    request->get("/categories/getCategories");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void Categories::remove(const QString &name) {
    QVariantMap params;
    params["name"] = name;
    Request *request = new Request(this);
    request->deleteResource("/categories/removeCategory", params);
    connect(request, SIGNAL(finished(this)), this, SLOT(onRequestFinished(Request*)));
}

void Categories::onRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        if (request->operation() == Request::GetOperation) {
            m_categories.clear();
            const QVariantList categories = request->result().toList();

            foreach (const QVariant &v, categories) {
                const QVariantMap c = v.toMap();
                m_categories << Category(c.value("name").toString(), c.value("path").toString());
            }

            emit changed();
        }
        else {
            load();
        }
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}
