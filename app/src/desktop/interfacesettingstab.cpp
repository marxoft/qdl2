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

#include "interfacesettingstab.h"
#include "settings.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

InterfaceSettingsTab::InterfaceSettingsTab(QWidget *parent) :
    SettingsTab(parent),
    m_webCheckBox(new QCheckBox(tr("&Enable web interface"), this)),
    m_authCheckBox(new QCheckBox(tr("Enable &authentication"), this)),
    m_usernameEdit(new QLineEdit(this)),
    m_passwordEdit(new QLineEdit(this)),
    m_portSpinBox(new QSpinBox(this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Interfaces"));

    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_portSpinBox->setMaximum(100000);

    m_layout->addRow(m_webCheckBox);
    m_layout->addRow(tr("&Port:"), m_portSpinBox);
    m_layout->addRow(m_authCheckBox);
    m_layout->addRow(tr("&Username:"), m_usernameEdit);
    m_layout->addRow(tr("&Password:"), m_passwordEdit);

    restore();
}

void InterfaceSettingsTab::restore() {
    m_webCheckBox->setChecked(Settings::webInterfaceEnabled());
    m_authCheckBox->setChecked(Settings::webInterfaceAuthenticationEnabled());
    m_usernameEdit->setText(Settings::webInterfaceUsername());
    m_passwordEdit->setText(Settings::webInterfacePassword());
    m_portSpinBox->setValue(Settings::webInterfacePort());
}

void InterfaceSettingsTab::save() {
    Settings::setWebInterfaceAuthenticationEnabled(m_authCheckBox->isChecked());
    Settings::setWebInterfaceUsername(m_usernameEdit->text());
    Settings::setWebInterfacePassword(m_passwordEdit->text());
    Settings::setWebInterfacePort(m_portSpinBox->value());
    Settings::setWebInterfaceEnabled(m_webCheckBox->isChecked());
}
