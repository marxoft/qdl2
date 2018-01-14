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

#include "networksettingspage.h"
#include "networkproxytypemodel.h"
#include "settings.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

NetworkSettingsPage::NetworkSettingsPage(QWidget *parent) :
    SettingsPage(parent),
    m_proxyTypeModel(new NetworkProxyTypeModel(this)),
    m_proxyCheckBox(new QCheckBox(tr("&Enable network proxy"), this)),
    m_authCheckBox(new QCheckBox(tr("Enable &authentication"), this)),
    m_proxyTypeSelector(new QComboBox(this)),
    m_hostEdit(new QLineEdit(this)),
    m_usernameEdit(new QLineEdit(this)),
    m_passwordEdit(new QLineEdit(this)),
    m_portSpinBox(new QSpinBox(this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Network"));

    m_proxyTypeSelector->setModel(m_proxyTypeModel);

    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_portSpinBox->setMaximum(100000);

    m_layout->addRow(m_proxyCheckBox);
    m_layout->addRow(m_proxyTypeSelector);
    m_layout->addRow(tr("&Host:"), m_hostEdit);
    m_layout->addRow(tr("&Port:"), m_portSpinBox);
    m_layout->addRow(m_authCheckBox);
    m_layout->addRow(tr("&Username:"), m_usernameEdit);
    m_layout->addRow(tr("&Password:"), m_passwordEdit);

    restore();
}

void NetworkSettingsPage::restore() {
    m_proxyCheckBox->setChecked(Settings::instance()->networkProxyEnabled());
    m_authCheckBox->setChecked(Settings::instance()->networkProxyAuthenticationEnabled());
    m_proxyTypeSelector->setCurrentIndex(qMax(0, m_proxyTypeSelector->findData(Settings::instance()->networkProxyType())));
    m_hostEdit->setText(Settings::instance()->networkProxyHost());
    m_usernameEdit->setText(Settings::instance()->networkProxyUsername());
    m_passwordEdit->setText(Settings::instance()->networkProxyPassword());
    m_portSpinBox->setValue(Settings::instance()->networkProxyPort());
}

void NetworkSettingsPage::save() {
    Settings::instance()->setNetworkProxyEnabled(m_proxyCheckBox->isChecked(), false);
    Settings::instance()->setNetworkProxyAuthenticationEnabled(m_authCheckBox->isChecked(), false);
    Settings::instance()->setNetworkProxyType(m_proxyTypeSelector->itemData(m_proxyTypeSelector->currentIndex()).toInt(), false);
    Settings::instance()->setNetworkProxyHost(m_hostEdit->text(), false);
    Settings::instance()->setNetworkProxyUsername(m_usernameEdit->text(), false);
    Settings::instance()->setNetworkProxyPassword(m_passwordEdit->text(), false);
    Settings::instance()->setNetworkProxyPort(m_portSpinBox->value(), false);
}
