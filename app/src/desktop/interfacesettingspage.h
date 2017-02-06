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

#ifndef INTERFACESETTINGSPAGE_H
#define INTERFACESETTINGSPAGE_H

#include "settingspage.h"

class QCheckBox;
class QFormLayout;
class QLineEdit;
class QSpinBox;

class InterfaceSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit InterfaceSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void restore();
    virtual void save();

private:
    QCheckBox *m_webCheckBox;
    QCheckBox *m_authCheckBox;

    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;

    QSpinBox *m_portSpinBox;

    QFormLayout *m_layout;
};

#endif // INTERFACESETTINGSPAGE_H
