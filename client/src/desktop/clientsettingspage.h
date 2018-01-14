/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Client Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Client Public License for more details.
 *
 * You should have received a copy of the GNU Client Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLIENTSETTINGSPAGE_H
#define CLIENTSETTINGSPAGE_H

#include "settingspage.h"

class QCheckBox;
class QFormLayout;
class QLineEdit;
class QSpinBox;

class ClientSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit ClientSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void restore();
    virtual void save();

private:
    QLineEdit *m_hostEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;

    QSpinBox *m_portSpinBox;

    QCheckBox *m_authCheckBox;
    QCheckBox *m_clipboardCheckBox;

    QFormLayout *m_layout;
};
    
#endif // CLIENTSETTINGSPAGE_H
