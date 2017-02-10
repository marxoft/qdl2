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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class DecaptchaPluginConfigModel;
class RecaptchaPluginConfigModel;
class SearchPluginConfigModel;
class ServicePluginConfigModel;
class QDialogButtonBox;
class QLabel;
class QListView;
class QTabBar;
class QVBoxLayout;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);

private Q_SLOTS:
    void showServicePlugins();
    void showRecaptchaPlugins();
    void showDecaptchaPlugins();
    void showSearchPlugins();
    
    void onTabChanged(int index);

private:
    ServicePluginConfigModel *m_serviceModel;
    RecaptchaPluginConfigModel *m_recaptchaModel;
    DecaptchaPluginConfigModel *m_decaptchaModel;
    SearchPluginConfigModel *m_searchModel;
    
    QLabel *m_iconLabel;
    QLabel *m_textLabel;

    QListView *m_view;

    QTabBar *m_tabBar;

    QDialogButtonBox *m_buttonBox;

    QVBoxLayout *m_layout;
};

#endif // ABOUTDIALOG_H
