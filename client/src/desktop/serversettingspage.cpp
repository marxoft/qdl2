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

#include "serversettingspage.h"
#include "categorysettingspage.h"
#include "decaptchasettingspage.h"
#include "generalsettingspage.h"
#include "networksettingspage.h"
#include "recaptchasettingspage.h"
#include "servicesettingspage.h"
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

ServerSettingsPage::ServerSettingsPage(QWidget *parent) :
    SettingsPage(parent),
    m_generalPage(0),
    m_networkPage(0),
    m_categoryPage(0),
    m_servicePage(0),
    m_recaptchaPage(0),
    m_decaptchaPage(0),
    m_tabBar(new QTabBar(this)),
    m_stack(new QStackedWidget(this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Server"));

    m_tabBar->setExpanding(false);
    m_tabBar->addTab(tr("General"));
    m_tabBar->addTab(tr("Network"));
    m_tabBar->addTab(tr("Categories"));
    m_tabBar->addTab(tr("Services"));
    m_tabBar->addTab(tr("Recaptcha"));
    m_tabBar->addTab(tr("Decaptcha"));

    m_layout->addWidget(m_tabBar);
    m_layout->addWidget(m_stack);

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(setCurrentPage(int)));

    showGeneralPage();
}

void ServerSettingsPage::save() {
    for (int i = 0; i < m_stack->count(); i++) {
        if (SettingsPage *page = qobject_cast<SettingsPage*>(m_stack->widget(i))) {
            page->save();
        }
    }
}

void ServerSettingsPage::setCurrentPage(int index) {
    switch (index) {
    case 0:
        showGeneralPage();
        break;
    case 1:
        showNetworkPage();
        break;
    case 2:
        showCategoryPage();
        break;
    case 3:
        showServicesPage();
        break;
    case 4:
        showRecaptchaPage();
        break;
    case 5:
        showDecaptchaPage();
        break;
    default:
        break;
    }
}

void ServerSettingsPage::showGeneralPage() {
    if (!m_generalPage) {
        m_generalPage = new GeneralSettingsPage(m_stack);
        m_stack->addWidget(m_generalPage);
    }

    m_stack->setCurrentWidget(m_generalPage);
}

void ServerSettingsPage::showNetworkPage() {
    if (!m_networkPage) {
        m_networkPage = new NetworkSettingsPage(m_stack);
        m_stack->addWidget(m_networkPage);
    }

    m_stack->setCurrentWidget(m_networkPage);
}

void ServerSettingsPage::showCategoryPage() {
    if (!m_categoryPage) {
        m_categoryPage = new CategorySettingsPage(m_stack);
        m_stack->addWidget(m_categoryPage);
    }

    m_stack->setCurrentWidget(m_categoryPage);
}

void ServerSettingsPage::showServicesPage() {
    if (!m_servicePage) {
        m_servicePage = new ServiceSettingsPage(m_stack);
        m_stack->addWidget(m_servicePage);
    }

    m_stack->setCurrentWidget(m_servicePage);
}

void ServerSettingsPage::showRecaptchaPage() {
    if (!m_recaptchaPage) {
        m_recaptchaPage = new RecaptchaSettingsPage(m_stack);
        m_stack->addWidget(m_recaptchaPage);
    }

    m_stack->setCurrentWidget(m_recaptchaPage);
}

void ServerSettingsPage::showDecaptchaPage() {
    if (!m_decaptchaPage) {
        m_decaptchaPage = new DecaptchaSettingsPage(m_stack);
        m_stack->addWidget(m_decaptchaPage);
    }

    m_stack->setCurrentWidget(m_decaptchaPage);
}
