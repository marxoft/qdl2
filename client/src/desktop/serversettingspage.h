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

#ifndef SERVERSETTINGSPAGE_H
#define SERVERSETTINGSPAGE_H

#include "settingspage.h"

class CategorySettingsPage;
class DecaptchaSettingsPage;
class GeneralSettingsPage;
class NetworkSettingsPage;
class RecaptchaSettingsPage;
class ServiceSettingsPage;
class QStackedWidget;
class QTabBar;
class QVBoxLayout;

class ServerSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit ServerSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void save();

private Q_SLOTS:
    void setCurrentPage(int index);
    
    void showGeneralPage();
    void showNetworkPage();
    void showCategoryPage();
    void showServicesPage();
    void showRecaptchaPage();
    void showDecaptchaPage();

private:
    GeneralSettingsPage *m_generalPage;
    NetworkSettingsPage *m_networkPage;
    CategorySettingsPage *m_categoryPage;
    ServiceSettingsPage *m_servicePage;
    RecaptchaSettingsPage *m_recaptchaPage;
    DecaptchaSettingsPage *m_decaptchaPage;
    
    QTabBar *m_tabBar;

    QStackedWidget *m_stack;

    QVBoxLayout *m_layout;
};

#endif // SERVERSETTINGSPAGE_H
