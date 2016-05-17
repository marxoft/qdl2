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

#include "recaptchadialog.h"
#include "plugindialog.h"
#include "recaptchapluginconfigmodel.h"
#include <QHBoxLayout>
#include <QListView>
#include <QMaemo5InformationBox>

RecaptchaDialog::RecaptchaDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new RecaptchaPluginConfigModel(this)),
    m_view(new QListView(this)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Recaptcha"));
    setMinimumHeight(360);

    m_view->setModel(m_model);
    m_view->setUniformItemSizes(true);
    
    m_layout->addWidget(m_view);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(showPluginDialog(QModelIndex)));
}

void RecaptchaDialog::showPluginDialog(const QModelIndex &index) {
    const QString id = index.data(RecaptchaPluginConfigModel::IdRole).toString();
    const QVariantList settings = index.data(RecaptchaPluginConfigModel::SettingsRole).toList();

    if ((id.isEmpty()) || (settings.isEmpty())) {
        QMaemo5InformationBox::information(this, tr("No settings for this plugin"));
        return;
    }

    PluginDialog dialog(id, settings, this);
    dialog.setWindowTitle(index.data(RecaptchaPluginConfigModel::DisplayNameRole).toString());
    dialog.exec();
}
