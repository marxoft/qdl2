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

#include "decaptchasettingspage.h"
#include "decaptchapluginconfigmodel.h"
#include "pluginsettingspage.h"
#include "settings.h"
#include <QCheckBox>
#include <QLabel>
#include <QListView>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>

DecaptchaSettingsPage::DecaptchaSettingsPage(QWidget *parent) :
    SettingsPage(parent),
    m_model(new DecaptchaPluginConfigModel(this)),
    m_view(new QListView(this)),
    m_container(new QWidget(this)),
    m_checkBox(new QCheckBox(tr("&Use this decaptcha plugin"), m_container)),
    m_scrollArea(new QScrollArea(m_container)),
    m_vbox(new QVBoxLayout(m_container)),
    m_splitter(new QSplitter(Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Decaptcha"));

    m_view->setModel(m_model);
    m_view->setUniformItemSizes(true);

    m_checkBox->hide();

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(new QLabel(tr("No plugin selected"), m_scrollArea));

    m_vbox->addWidget(m_checkBox);
    m_vbox->addWidget(m_scrollArea);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_splitter->addWidget(m_view);
    m_splitter->addWidget(m_container);
    m_splitter->setStretchFactor(1, 1);

    m_layout->addWidget(m_splitter);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentPlugin(QModelIndex)));
}

void DecaptchaSettingsPage::save() {    
    if (PluginSettingsPage *page = qobject_cast<PluginSettingsPage*>(m_scrollArea->widget())) {
        page->save();
        Settings::instance()->setDecaptchaPlugin(m_checkBox->isChecked() ? m_pluginId : QString());
    }    
}

void DecaptchaSettingsPage::setCurrentPlugin(const QModelIndex &index) {
    save();
    
    if (!index.isValid()) {
        m_scrollArea->setWidget(new QLabel(tr("No plugin selected"), m_scrollArea));
        return;
    }

    m_pluginId = index.data(DecaptchaPluginConfigModel::IdRole).toString();

    if (m_pluginId.isEmpty()) {
        m_checkBox->hide();
        m_scrollArea->setWidget(new QLabel(tr("No settings for this plugin"), m_scrollArea));
        return;
    }

    m_checkBox->show();
    m_checkBox->setChecked(Settings::instance()->decaptchaPlugin() == m_pluginId);
    m_scrollArea->setWidget(new PluginSettingsPage(m_pluginId, m_scrollArea));
}
