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

#include "settingsdialog.h"
#include "archivepasswordsdialog.h"
#include "categoriesdialog.h"
#include "concurrenttransfersmodel.h"
#include "decaptchadialog.h"
#include "networkproxydialog.h"
#include "recaptchadialog.h"
#include "servicesdialog.h"
#include "settings.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_concurrentModel(new ConcurrentTransfersModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_downloadPathButton(new QMaemo5ValueButton(tr("Default download path"), m_container)),
    m_concurrentSelector(new ValueSelector(tr("Maximum concurrent downloads"), m_container)),
    m_automaticCheckBox(new QCheckBox(tr("Start downloads automatically"), m_container)),
    m_subfoldersCheckBox(new QCheckBox(tr("Create subfolders for downloads"), m_container)),
    m_clipboardCheckBox(new QCheckBox(tr("Monitor clipboard for URLs"), m_container)),
    m_extractCheckBox(new QCheckBox(tr("Extract archives"), m_container)),
    m_deleteCheckBox(new QCheckBox(tr("Delete extracted archives"), m_container)),
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

    m_downloadPathButton->setValueText(Settings::downloadPath());
    m_concurrentSelector->setModel(m_concurrentModel);
    m_concurrentSelector->setValue(Settings::maximumConcurrentTransfers());
    m_automaticCheckBox->setChecked(Settings::startTransfersAutomatically());
    m_subfoldersCheckBox->setChecked(Settings::createSubfolders());
    m_clipboardCheckBox->setChecked(Settings::clipboardMonitorEnabled());
    m_extractCheckBox->setChecked(Settings::extractArchives());
    m_deleteCheckBox->setChecked(Settings::deleteExtractedArchives());

    const QString midColor = palette().color(QPalette::Mid).name();

    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("General")), m_container));
    m_vbox->addWidget(m_downloadPathButton);
    m_vbox->addWidget(m_concurrentSelector);
    m_vbox->addWidget(m_automaticCheckBox);
    m_vbox->addWidget(m_subfoldersCheckBox);
    m_vbox->addWidget(m_clipboardCheckBox);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Archives")), m_container));
    m_vbox->addWidget(m_extractCheckBox);
    m_vbox->addWidget(m_deleteCheckBox);
    m_vbox->addWidget(m_passwordsButton);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Network")), m_container));
    m_vbox->addWidget(m_proxyButton);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Categories")), m_container));
    m_vbox->addWidget(m_categoriesButton);
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(midColor).arg(tr("Plugins")), m_container));
    m_vbox->addWidget(m_servicesButton);
    m_vbox->addWidget(m_recaptchaButton);
    m_vbox->addWidget(m_decaptchaButton);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);

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
    Settings::setDownloadPath(m_downloadPathButton->valueText());
    Settings::setMaximumConcurrentTransfers(m_concurrentSelector->currentValue().toInt());
    Settings::setStartTransfersAutomatically(m_automaticCheckBox->isChecked());
    Settings::setCreateSubfolders(m_subfoldersCheckBox->isChecked());
    Settings::setClipboardMonitorEnabled(m_clipboardCheckBox->isChecked());
    Settings::setExtractArchives(m_extractCheckBox->isChecked());
    Settings::setDeleteExtractedArchives(m_deleteCheckBox->isChecked());
    QDialog::accept();
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
    ServicesDialog(this).exec();
}

void SettingsDialog::showRecaptchaDialog() {
    RecaptchaDialog(this).exec();
}

void SettingsDialog::showDecaptchaDialog() {
    DecaptchaDialog(this).exec();
}
