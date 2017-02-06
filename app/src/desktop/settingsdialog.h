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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class CategorySettingsPage;
class DecaptchaSettingsPage;
class GeneralSettingsPage;
class InterfaceSettingsPage;
class NetworkSettingsPage;
class RecaptchaSettingsPage;
class ServiceSettingsPage;
class QDialogButtonBox;
class QStackedWidget;
class QTabBar;
class QVBoxLayout;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);

public Q_SLOTS:
    virtual void accept();

private Q_SLOTS:
    void setCurrentPage(int index);
    
    void showGeneralPage();
    void showNetworkPage();
    void showInterfacePage();
    void showCategoryPage();
    void showServicesPage();
    void showRecaptchaPage();
    void showDecaptchaPage();

private:
    GeneralSettingsPage *m_generalPage;
    NetworkSettingsPage *m_networkPage;
    InterfaceSettingsPage *m_interfacePage;
    CategorySettingsPage *m_categoryPage;
    ServiceSettingsPage *m_servicePage;
    RecaptchaSettingsPage *m_recaptchaPage;
    DecaptchaSettingsPage *m_decaptchaPage;
    
    QTabBar *m_tabBar;

    QStackedWidget *m_stack;

    QDialogButtonBox *m_buttonBox;

    QVBoxLayout *m_layout;
};

#endif // SETTINGSDIALOG_H
