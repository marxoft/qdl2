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

#ifndef URLACTIONMODEL_H
#define URLACTIONMODEL_H

#include "selectionmodel.h"
#include "qdl.h"

class UrlActionModel : public SelectionModel
{
    Q_OBJECT

public:
    explicit UrlActionModel(QObject *parent = 0) :
        SelectionModel(parent)
    {
        append(tr("Add URLs"), Qdl::AddUrls);
        append(tr("Retrieve URLs"), Qdl::RetrieveUrls);
    }
};

#endif // URLACTIONMODEL_H
