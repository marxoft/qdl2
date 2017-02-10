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

#include "aboutpluginsdialog.h"
#include "decaptchapluginconfigmodel.h"
#include "recaptchapluginconfigmodel.h"
#include "searchpluginconfigmodel.h"
#include "servicepluginconfigmodel.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QListView>
#include <QMaemo5InformationBox>
#include <QPushButton>
#include <QTabBar>

AboutPluginsDialog::AboutPluginsDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(0),
    m_recaptchaModel(0),
    m_decaptchaModel(0),
    m_searchModel(0),
    m_view(new QListView(this)),
    m_tabBar(new QTabBar(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("Installed plugins"));
    setMinimumHeight(360);
    
    m_view->setUniformItemSizes(true);
    m_view->setSelectionMode(QListView::NoSelection);
    
    m_tabBar->addTab(tr("Services"));
    m_tabBar->addTab(tr("Recaptcha"));
    m_tabBar->addTab(tr("Decaptcha"));
    m_tabBar->addTab(tr("Search"));
    
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Load"));

    m_layout->addWidget(m_tabBar, 0, 0);
    m_layout->addWidget(m_view, 1, 0);
    m_layout->addWidget(m_buttonBox, 1, 1);
    m_layout->setRowStretch(1, 1);

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(loadPlugins()));

    showServicePlugins();
}

void AboutPluginsDialog::loadPlugins() {
    const int decaptcha = DecaptchaPluginManager::instance()->load();
    
    if ((decaptcha > 0) && (m_decaptchaModel)) {
        m_decaptchaModel->reload();
    }
    
    const int recaptcha = DecaptchaPluginManager::instance()->load();
    
    if ((recaptcha > 0) && (m_recaptchaModel)) {
        m_recaptchaModel->reload();
    }
    
    const int services = ServicePluginManager::instance()->load();
    
    if ((services > 0) && (m_serviceModel)) {
        m_serviceModel->reload();
    }
    
    const int count = decaptcha + recaptcha + services;

    if (count > 0) {
        QMaemo5InformationBox::information(this, tr("%1 new plugins found").arg(count));        
    }
    else {
        QMaemo5InformationBox::information(this, tr("No new plugins found"));
    }
}

void AboutPluginsDialog::showServicePlugins() {
    if (!m_serviceModel) {
        m_serviceModel = new ServicePluginConfigModel(this);
    }

    m_view->setModel(m_serviceModel);
}

void AboutPluginsDialog::showRecaptchaPlugins() {
    if (!m_recaptchaModel) {
        m_recaptchaModel = new RecaptchaPluginConfigModel(this);
    }

    m_view->setModel(m_recaptchaModel);
}

void AboutPluginsDialog::showDecaptchaPlugins() {
    if (!m_decaptchaModel) {
        m_decaptchaModel = new DecaptchaPluginConfigModel(this);
    }

    m_view->setModel(m_decaptchaModel);
}

void AboutPluginsDialog::showSearchPlugins() {
    if (!m_searchModel) {
        m_searchModel = new SearchPluginConfigModel(this);
    }

    m_view->setModel(m_searchModel);
}

void AboutPluginsDialog::onTabChanged(int index) {
    switch (index) {
    case 0:
        showServicePlugins();
        break;
    case 1:
        showRecaptchaPlugins();
        break;
    case 2:
        showDecaptchaPlugins();
        break;
    case 3:
        showSearchPlugins();
        break;
    default:
        break;
    }
}
