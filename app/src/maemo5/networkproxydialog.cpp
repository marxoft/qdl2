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

#include "networkproxydialog.h"
#include "networkproxytypemodel.h"
#include "settings.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

NetworkProxyDialog::NetworkProxyDialog(QWidget *parent) :
    QDialog(parent),
    m_proxyTypeModel(new NetworkProxyTypeModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_proxyCheckBox(new QCheckBox(tr("Enable network proxy"), m_container)),
    m_authCheckBox(new QCheckBox(tr("Enable authentication"), m_container)),
    m_proxyTypeSelector(new ValueSelector(tr("Type"), m_container)),
    m_hostEdit(new QLineEdit(m_container)),
    m_usernameEdit(new QLineEdit(m_container)),
    m_passwordEdit(new QLineEdit(m_container)),
    m_portSpinBox(new QSpinBox(m_container)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Network proxy"));
    setMinimumHeight(360);

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);

    m_proxyCheckBox->setChecked(Settings::networkProxyEnabled());

    m_authCheckBox->setChecked(Settings::networkProxyAuthenticationEnabled());    
    
    m_proxyTypeSelector->setModel(m_proxyTypeModel);
    m_proxyTypeSelector->setValue(Settings::networkProxyType());    

    m_hostEdit->setText(Settings::networkProxyHost());

    m_usernameEdit->setText(Settings::networkProxyUsername());

    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setText(Settings::networkProxyPassword());

    m_portSpinBox->setMaximum(100000);
    m_portSpinBox->setValue(Settings::networkProxyPort());

    m_vbox->addWidget(m_proxyCheckBox);
    m_vbox->addWidget(m_proxyTypeSelector);
    m_vbox->addWidget(new QLabel(tr("Host"), m_container));
    m_vbox->addWidget(m_hostEdit);
    m_vbox->addWidget(new QLabel(tr("Port"), m_container));
    m_vbox->addWidget(m_portSpinBox);
    m_vbox->addWidget(m_authCheckBox);
    m_vbox->addWidget(new QLabel(tr("Username"), m_container));
    m_vbox->addWidget(m_usernameEdit);
    m_vbox->addWidget(new QLabel(tr("Password"), m_container));
    m_vbox->addWidget(m_passwordEdit);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void NetworkProxyDialog::accept() {
    const bool enabled = m_proxyCheckBox->isChecked();
    const bool authEnabled = m_authCheckBox->isChecked();
    const int type = m_proxyTypeSelector->currentValue().toInt();
    const QString host = m_hostEdit->text();
    const QString username = m_usernameEdit->text();
    const QString password = m_passwordEdit->text();
    const int port = m_portSpinBox->value();
    
    Settings::setNetworkProxyEnabled(enabled);
    Settings::setNetworkProxyAuthenticationEnabled(authEnabled);
    Settings::setNetworkProxyType(type);
    Settings::setNetworkProxyHost(host);
    Settings::setNetworkProxyUsername(username);
    Settings::setNetworkProxyPassword(password);
    Settings::setNetworkProxyPort(port);

    QNetworkProxy proxy;

    if (m_proxyCheckBox->isChecked()) {
        proxy.setHostName(host);
        proxy.setPort(port);
        proxy.setType(QNetworkProxy::ProxyType(type));

        if (authEnabled) {
            proxy.setUser(username);
            proxy.setPassword(password);
        }
    }

    QNetworkProxy::setApplicationProxy(proxy);
}
