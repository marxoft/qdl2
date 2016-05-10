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

#include "categories.h"
#include "definitions.h"
#include <QSettings>

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

bool Categories::add(const QString &name, const QString &path) {
    QSettings categories(APP_CONFIG_PATH + "categories", QSettings::IniFormat);

    if (path != categories.value(name)) {
        categories.setValue(name, path);

        if (self) {
            emit self->changed();
        }

        return true;
    }

    return false;
}

CategoryList Categories::get() {
    QSettings categories(APP_CONFIG_PATH + "categories", QSettings::IniFormat);
    CategoryList list;
    
    foreach (const QString &name, categories.childKeys()) {
        const QString path = categories.value(name).toString();
        list << Category(name, path.endsWith("/") ? path : path + "/");
    }

    return list;
}

Category Categories::get(const QString &name) {
    const QString path = QSettings(APP_CONFIG_PATH + "categories", QSettings::IniFormat).value(name).toString();

    if (path.isEmpty()) {
        return Category();
    }
    
    return Category(name, path.endsWith("/") ? path : path + "/");
}

bool Categories::remove(const QString &name) {
    QSettings categories(APP_CONFIG_PATH + "categories", QSettings::IniFormat);

    if (categories.contains(name)) {
        categories.remove(name);

        if (self) {
            emit self->changed();
        }

        return true;
    }

    return false;
}
