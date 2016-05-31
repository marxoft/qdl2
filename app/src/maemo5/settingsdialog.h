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

class ConcurrentTransfersModel;
class ValueSelector;
class QCheckBox;
class QDialogButtonBox;
class QHBoxLayout;
class QLineEdit;
class QMaemo5ValueButton;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);

public Q_SLOTS:
    virtual void accept();

private Q_SLOTS:
    void showFileDialog();
    void showArchivePasswordsDialog();
    void showNetworkProxyDialog();
    void showCategoriesDialog();
    void showServicesDialog();
    void showRecaptchaDialog();
    void showDecaptchaDialog();

private:
    ConcurrentTransfersModel *m_concurrentModel;
    
    QScrollArea *m_scrollArea;

    QWidget *m_container;
    
    QMaemo5ValueButton *m_downloadPathButton;

    ValueSelector *m_concurrentSelector;
    
    QLineEdit *m_commandEdit;
    
    QCheckBox *m_commandCheckBox;
    QCheckBox *m_automaticCheckBox;
    QCheckBox *m_subfoldersCheckBox;
    QCheckBox *m_clipboardCheckBox;
    QCheckBox *m_extractCheckBox;
    QCheckBox *m_deleteCheckBox;

    QPushButton *m_passwordsButton;
    QPushButton *m_proxyButton;
    QPushButton *m_categoriesButton;
    QPushButton *m_servicesButton;
    QPushButton *m_recaptchaButton;
    QPushButton *m_decaptchaButton;

    QDialogButtonBox *m_buttonBox;

    QVBoxLayout *m_vbox;
    QHBoxLayout *m_layout;
};

#endif // SETTINGSDIALOG_H
