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

#include "settingsdialog.h"
#include "archivepasswordsdialog.h"
#include "categoriesdialog.h"
#include "connectiondialog.h"
#include "concurrenttransfersmodel.h"
#include "decaptchapluginsdialog.h"
#include "networkproxydialog.h"
#include "recaptchapluginsdialog.h"
#include "servicepluginsdialog.h"
#include "settings.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_concurrentModel(new ConcurrentTransfersModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_downloadPathButton(new QMaemo5ValueButton(tr("Default download path"), m_container)),
    m_concurrentSelector(new ValueSelector(tr("Maximum concurrent downloads"), m_container)),
    m_commandEdit(new QLineEdit(Settings::instance()->customCommand(), m_container)),
    m_clipboardCheckBox(new QCheckBox(tr("Monitor clipboard for URLs"), m_container)),
    m_commandCheckBox(new QCheckBox(tr("Enable custom command"), m_container)),
    m_automaticCheckBox(new QCheckBox(tr("Start downloads automatically"), m_container)),
    m_extractCheckBox(new QCheckBox(tr("Extract archives"), m_container)),
    m_deleteCheckBox(new QCheckBox(tr("Delete extracted archives"), m_container)),
    m_connectionButton(new QPushButton(tr("Server connection"), m_container)),
    m_passwordsButton(new QPushButton(tr("Archive passwords"), m_container)),
    m_proxyButton(new QPushButton(tr("Network proxy"), m_container)),
    m_categoriesButton(new QPushButton(tr("Categories"), m_container)),
    m_servicesButton(new QPushButton(tr("Services"), m_container)),
    m_recaptchaButton(new QPushButton(tr("Recaptcha"), m_container)),
    m_decaptchaButton(new QPushButton(tr("Decaptcha"), m_container)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Settings"));
    setMinimumHeight(360);

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);

    m_downloadPathButton->setValueText(Settings::instance()->downloadPath());
    m_concurrentSelector->setModel(m_concurrentModel);
    m_concurrentSelector->setValue(Settings::instance()->maximumConcurrentTransfers());
    m_clipboardCheckBox->setChecked(Settings::clipboardMonitorEnabled());
    m_commandCheckBox->setChecked(Settings::instance()->customCommandEnabled());
    m_automaticCheckBox->setChecked(Settings::instance()->startTransfersAutomatically());
    m_extractCheckBox->setChecked(Settings::instance()->extractArchives());
    m_deleteCheckBox->setChecked(Settings::instance()->deleteExtractedArchives());

    const QString midColor = palette().color(QPalette::Mid).name();

    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Client")), m_container));
    m_vbox->addWidget(m_connectionButton);
    m_vbox->addWidget(m_clipboardCheckBox);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Server - General")), m_container));
    m_vbox->addWidget(m_downloadPathButton);
    m_vbox->addWidget(m_concurrentSelector);
    m_vbox->addWidget(m_automaticCheckBox);
    m_vbox->addWidget(new QLabel(tr("Custom command (%f for filename)"), m_container));
    m_vbox->addWidget(m_commandEdit);
    m_vbox->addWidget(m_commandCheckBox);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Server - Archives")), m_container));
    m_vbox->addWidget(m_extractCheckBox);
    m_vbox->addWidget(m_deleteCheckBox);
    m_vbox->addWidget(m_passwordsButton);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Server - Network")), m_container));
    m_vbox->addWidget(m_proxyButton);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Server - Categories")), m_container));
    m_vbox->addWidget(m_categoriesButton);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Server - Plugins")), m_container));
    m_vbox->addWidget(m_servicesButton);
    m_vbox->addWidget(m_recaptchaButton);
    m_vbox->addWidget(m_decaptchaButton);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);

    connect(m_connectionButton, SIGNAL(clicked()), this, SLOT(showConnectionDialog()));
    connect(m_downloadPathButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    connect(m_passwordsButton, SIGNAL(clicked()), this, SLOT(showArchivePasswordsDialog()));
    connect(m_proxyButton, SIGNAL(clicked()), this, SLOT(showNetworkProxyDialog()));
    connect(m_categoriesButton, SIGNAL(clicked()), this, SLOT(showCategoriesDialog()));
    connect(m_servicesButton, SIGNAL(clicked()), this, SLOT(showServicesDialog()));
    connect(m_recaptchaButton, SIGNAL(clicked()), this, SLOT(showRecaptchaDialog()));
    connect(m_decaptchaButton, SIGNAL(clicked()), this, SLOT(showDecaptchaDialog()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void SettingsDialog::accept() {
    Settings::setClipboardMonitorEnabled(m_clipboardCheckBox->isChecked());    
    Settings::instance()->setDownloadPath(m_downloadPathButton->valueText(), false);
    Settings::instance()->setMaximumConcurrentTransfers(m_concurrentSelector->currentValue().toInt(), false);
    Settings::instance()->setCustomCommand(m_commandEdit->text(), false);
    Settings::instance()->setCustomCommandEnabled(m_commandCheckBox->isChecked(), false);
    Settings::instance()->setStartTransfersAutomatically(m_automaticCheckBox->isChecked(), false);
    Settings::instance()->setExtractArchives(m_extractCheckBox->isChecked(), false);
    Settings::instance()->setDeleteExtractedArchives(m_deleteCheckBox->isChecked(), false);
    Settings::instance()->save();
    QDialog::accept();
}

void SettingsDialog::showConnectionDialog() {
    ConnectionDialog(this).exec();
}

void SettingsDialog::showFileDialog() {
    const QString path = QFileDialog::getExistingDirectory(this, tr("Download path"),
                                                           m_downloadPathButton->valueText());

    if (!path.isEmpty()) {
        m_downloadPathButton->setValueText(path);
    }
}

void SettingsDialog::showArchivePasswordsDialog() {
    ArchivePasswordsDialog(this).exec();
}

void SettingsDialog::showNetworkProxyDialog() {
    NetworkProxyDialog(this).exec();
}

void SettingsDialog::showCategoriesDialog() {
    CategoriesDialog(this).exec();
}

void SettingsDialog::showServicesDialog() {
    ServicePluginsDialog(this).exec();
}

void SettingsDialog::showRecaptchaDialog() {
    RecaptchaPluginsDialog(this).exec();
}

void SettingsDialog::showDecaptchaDialog() {
    DecaptchaPluginsDialog(this).exec();
}
