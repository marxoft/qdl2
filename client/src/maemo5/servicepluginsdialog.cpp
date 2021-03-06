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

#include "servicepluginsdialog.h"
#include "plugindialog.h"
#include "servicepluginconfigmodel.h"
#include <QHBoxLayout>
#include <QListView>
#include <QMaemo5InformationBox>

ServicePluginsDialog::ServicePluginsDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new ServicePluginConfigModel(this)),
    m_view(new QListView(this)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Services"));
    setMinimumHeight(360);

    m_view->setModel(m_model);
    m_view->setUniformItemSizes(true);
    
    m_layout->addWidget(m_view);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(showPluginDialog(QModelIndex)));
}

void ServicePluginsDialog::showPluginDialog(const QModelIndex &index) {
    const QString id = index.data(ServicePluginConfigModel::IdRole).toString();

    if (id.isEmpty()) {
        QMaemo5InformationBox::information(this, tr("No settings for this plugin"));
        return;
    }

    PluginDialog dialog(id, this);
    dialog.setWindowTitle(index.data(ServicePluginConfigModel::DisplayNameRole).toString());
    dialog.exec();
}
