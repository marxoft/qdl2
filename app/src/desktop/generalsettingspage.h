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

#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include "settingspage.h"

class ArchivePasswordModel;
class QCheckBox;
class QFormLayout;
class QLineEdit;
class QListView;
class QPushButton;
class QSpinBox;

class GeneralSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit GeneralSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void restore();
    virtual void save();

private Q_SLOTS:
    void showFileDialog();
    
    void addPassword();
    
    void showPasswordContextMenu(const QPoint &pos);
    
    void onPasswordChanged(const QString &password);
    
private:
    ArchivePasswordModel *m_passwordModel;
    
    QLineEdit *m_pathEdit;
    QLineEdit *m_commandEdit;
    QLineEdit *m_passwordEdit;

    QPushButton *m_pathButton;
    QPushButton *m_passwordButton;

    QSpinBox *m_concurrentSpinBox;
    
    QCheckBox *m_commandCheckBox;
    QCheckBox *m_clipboardCheckBox;
    QCheckBox *m_extractCheckBox;
    QCheckBox *m_deleteCheckBox;

    QListView *m_passwordView;

    QFormLayout *m_layout;
};
    
#endif // GENERALSETTINGSPAGE_H
