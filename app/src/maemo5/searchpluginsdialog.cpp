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

#include "searchpluginsdialog.h"
#include "plugindialog.h"
#include "searchpluginconfigmodel.h"
#include <QHBoxLayout>
#include <QListView>
#include <QMaemo5InformationBox>

SearchPluginsDialog::SearchPluginsDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new SearchPluginConfigModel(this)),
    m_view(new QListView(this)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Search"));
    setMinimumHeight(360);

    m_view->setModel(m_model);
    m_view->setUniformItemSizes(true);
    
    m_layout->addWidget(m_view);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(showPluginDialog(QModelIndex)));
}

void SearchPluginsDialog::showPluginDialog(const QModelIndex &index) {
    const QString id = index.data(SearchPluginConfigModel::IdRole).toString();
    const QVariantList settings = index.data(SearchPluginConfigModel::SettingsRole).toList();

    if ((id.isEmpty()) || (settings.isEmpty())) {
        QMaemo5InformationBox::information(this, tr("No settings for this plugin"));
        return;
    }

    PluginDialog dialog(id, settings, this);
    dialog.setWindowTitle(index.data(SearchPluginConfigModel::DisplayNameRole).toString());
    dialog.exec();
}
