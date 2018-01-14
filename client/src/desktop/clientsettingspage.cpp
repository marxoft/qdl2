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

#include "clientsettingspage.h"
#include "settings.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

ClientSettingsPage::ClientSettingsPage(QWidget *parent) :
    SettingsPage(parent),
    m_hostEdit(new QLineEdit(this)),
    m_usernameEdit(new QLineEdit(this)),
    m_passwordEdit(new QLineEdit(this)),
    m_portSpinBox(new QSpinBox(this)),
    m_authCheckBox(new QCheckBox(tr("Enable server &authentication"), this)),
    m_clipboardCheckBox(new QCheckBox(tr("Monitor &clipboard for URLs"), this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Client"));

    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_portSpinBox->setMaximum(100000);

    m_layout->addRow(tr("Server &host:"), m_hostEdit);
    m_layout->addRow(tr("Server &port:"), m_portSpinBox);
    m_layout->addRow(m_authCheckBox);
    m_layout->addRow(tr("Server &username:"), m_usernameEdit);
    m_layout->addRow(tr("Server &password:"), m_passwordEdit);
    m_layout->addRow(m_clipboardCheckBox);

    restore();
}

void ClientSettingsPage::restore() {
    m_hostEdit->setText(Settings::serverHost());
    m_usernameEdit->setText(Settings::serverUsername());
    m_passwordEdit->setText(Settings::serverPassword());
    m_portSpinBox->setValue(Settings::serverPort());
    m_authCheckBox->setChecked(Settings::serverAuthenticationEnabled());
    m_clipboardCheckBox->setChecked(Settings::clipboardMonitorEnabled());
}

void ClientSettingsPage::save() {
    Settings::setServerHost(m_hostEdit->text());
    Settings::setServerUsername(m_usernameEdit->text());
    Settings::setServerPassword(m_passwordEdit->text());
    Settings::setServerPort(m_portSpinBox->value());
    Settings::setServerAuthenticationEnabled(m_authCheckBox->isChecked());
    Settings::setClipboardMonitorEnabled(m_clipboardCheckBox->isChecked());
}
