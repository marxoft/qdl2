/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SEARCHSELECTIONMODEL_H
#define SEARCHSELECTIONMODEL_H

#include "selectionmodel.h"
#include "searchpluginmanager.h"

class SearchSelectionModel : public SelectionModel
{
    Q_OBJECT

public:
    explicit SearchSelectionModel(QObject *parent = 0) :
        SelectionModel(parent)
    {
        reload();
    }

public Q_SLOTS:
    void reload() {
        clear();
        
        foreach (const SearchPluginPair &pair, SearchPluginManager::instance()->plugins()) {
            append(pair.config->displayName(), pair.config->id());
        }
    }
};

#endif // SEARCHSELECTIONMODEL_H
