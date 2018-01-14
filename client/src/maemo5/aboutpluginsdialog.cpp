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

#include "aboutpluginsdialog.h"
#include "decaptchapluginconfigmodel.h"
#include "recaptchapluginconfigmodel.h"
#include "servicepluginconfigmodel.h"
#include <QDialogButtonBox>
#include <QListView>
#include <QMaemo5InformationBox>
#include <QPushButton>
#include <QTabBar>
#include <QVBoxLayout>

AboutPluginsDialog::AboutPluginsDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(0),
    m_recaptchaModel(0),
    m_decaptchaModel(0),
    m_view(new QListView(this)),
    m_tabBar(new QTabBar(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Installed plugins"));
    setMinimumHeight(360);
    
    m_view->setUniformItemSizes(true);
    m_view->setSelectionMode(QListView::NoSelection);
    
    m_tabBar->setExpanding(false);
    m_tabBar->setStyleSheet("QTabBar::tab { height: 40px; }");
    m_tabBar->addTab(tr("Services"));
    m_tabBar->addTab(tr("Recaptcha"));
    m_tabBar->addTab(tr("Decaptcha"));
    
    m_layout->addWidget(m_tabBar);
    m_layout->addWidget(m_view);
    m_layout->setStretch(1, 1);

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    showServicePlugins();
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
    default:
        break;
    }
}
