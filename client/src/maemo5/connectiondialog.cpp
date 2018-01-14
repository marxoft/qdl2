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

#include "connectiondialog.h"
#include "settings.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_authCheckBox(new QCheckBox(tr("Enable authentication"), m_container)),
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

    m_authCheckBox->setChecked(Settings::serverAuthenticationEnabled());    
    
    m_hostEdit->setText(Settings::serverHost());

    m_usernameEdit->setText(Settings::serverUsername());

    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setText(Settings::serverPassword());

    m_portSpinBox->setMaximum(100000);
    m_portSpinBox->setValue(Settings::serverPort());

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

void ConnectionDialog::accept() {
    Settings::setServerHost(m_hostEdit->text());
    Settings::setServerPort(m_portSpinBox->value());
    Settings::setServerAuthenticationEnabled(m_authCheckBox->isChecked());
    Settings::setServerUsername(m_usernameEdit->text());
    Settings::setServerPassword(m_passwordEdit->text());
    QDialog::accept();
}
