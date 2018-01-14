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

#include "decaptchapluginsdialog.h"
#include "decaptchapluginconfigmodel.h"
#include "plugindialog.h"
#include "settings.h"
#include <QAction>
#include <QHBoxLayout>
#include <QListView>
#include <QMaemo5InformationBox>
#include <QMenu>

DecaptchaPluginsDialog::DecaptchaPluginsDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new DecaptchaPluginConfigModel(this)),
    m_view(new QListView(this)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Decaptcha"));
    setMinimumHeight(360);

    m_view->setModel(m_model);
    m_view->setUniformItemSizes(true);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_layout->addWidget(m_view);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(showPluginDialog(QModelIndex)));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void DecaptchaPluginsDialog::showContextMenu(const QPoint &pos) {
    if (!m_view->currentIndex().isValid()) {
        return;
    }

    const QString id = m_view->currentIndex().data(DecaptchaPluginConfigModel::IdRole).toString();

    QMenu menu(this);
    QAction *action = menu.addAction(tr("Use this decaptcha plugin"));
    action->setCheckable(true);
    action->setChecked(Settings::instance()->decaptchaPlugin() == id);

    if (menu.exec(mapToGlobal(pos))) {
        Settings::instance()->setDecaptchaPlugin(action->isChecked() ? id : QString());
    }
}

void DecaptchaPluginsDialog::showPluginDialog(const QModelIndex &index) {
    const QString id = index.data(DecaptchaPluginConfigModel::IdRole).toString();

    if (id.isEmpty()) {
        QMaemo5InformationBox::information(this, tr("No settings for this plugin"));
        return;
    }

    PluginDialog dialog(id, this);
    dialog.setWindowTitle(index.data(DecaptchaPluginConfigModel::DisplayNameRole).toString());
    dialog.exec();
}
