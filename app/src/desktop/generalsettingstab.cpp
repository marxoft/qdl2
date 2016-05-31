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

#include "generalsettingstab.h"
#include "archivepasswordmodel.h"
#include "definitions.h"
#include "settings.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>

GeneralSettingsTab::GeneralSettingsTab(QWidget *parent) :
    SettingsTab(parent),
    m_passwordModel(new ArchivePasswordModel(this)),
    m_pathEdit(new QLineEdit(this)),
    m_commandEdit(new QLineEdit(this)),
    m_passwordEdit(new QLineEdit(this)),
    m_pathButton(new QPushButton(QIcon::fromTheme("document-open"), tr("&Browse"), this)),
    m_passwordButton(new QPushButton(QIcon::fromTheme("list-add"), tr("&Add"), this)),
    m_concurrentSpinBox(new QSpinBox(this)),
    m_commandCheckBox(new QCheckBox(tr("&Enable custom command"), this)),
    m_automaticCheckBox(new QCheckBox(tr("Start downloads &automatically"), this)),
    m_subfoldersCheckBox(new QCheckBox(tr("Create &subfolders for downloads"), this)),
    m_clipboardCheckBox(new QCheckBox(tr("Monitor &clipboard for URLs"), this)),
    m_extractCheckBox(new QCheckBox(tr("&Extract archives"), this)),
    m_deleteCheckBox(new QCheckBox(tr("&Delete extracted archives"), this)),
    m_passwordView(new QListView(this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("General"));

    m_passwordButton->setEnabled(false);

    m_concurrentSpinBox->setRange(1, MAX_CONCURRENT_TRANSFERS);

    m_passwordView->setModel(m_passwordModel);
    m_passwordView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_layout->addRow(tr("Download &path:"), m_pathEdit);
    m_layout->addWidget(m_pathButton);
    m_layout->addRow(tr("&Maximum concurrent downloads:"), m_concurrentSpinBox);
    m_layout->addRow(tr("&Custom command (%f for filename):"), m_commandEdit);
    m_layout->addRow(m_commandCheckBox);
    m_layout->addRow(m_automaticCheckBox);
    m_layout->addRow(m_subfoldersCheckBox);
    m_layout->addRow(m_clipboardCheckBox);
    m_layout->addRow(m_extractCheckBox);
    m_layout->addRow(m_deleteCheckBox);
    m_layout->addRow(new QLabel(tr("Archive passwords:"), this));
    m_layout->addRow(m_passwordView);
    m_layout->addRow(tr("Add &password:"), m_passwordEdit);
    m_layout->addWidget(m_passwordButton);

    connect(m_passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordChanged(QString)));
    connect(m_passwordEdit, SIGNAL(returnPressed()), m_passwordButton, SLOT(animateClick()));
    connect(m_pathButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    connect(m_passwordButton, SIGNAL(clicked()), this, SLOT(addPassword()));
    connect(m_passwordView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPasswordContextMenu(QPoint)));

    restore();
}

void GeneralSettingsTab::restore() {
    m_pathEdit->setText(Settings::downloadPath());
    m_concurrentSpinBox->setValue(Settings::maximumConcurrentTransfers());
    m_commandEdit->setText(Settings::customCommand());
    m_commandCheckBox->setChecked(Settings::customCommandEnabled());
    m_automaticCheckBox->setChecked(Settings::startTransfersAutomatically());
    m_subfoldersCheckBox->setChecked(Settings::createSubfolders());
    m_clipboardCheckBox->setChecked(Settings::clipboardMonitorEnabled());
    m_extractCheckBox->setChecked(Settings::extractArchives());
    m_deleteCheckBox->setChecked(Settings::deleteExtractedArchives());
}

void GeneralSettingsTab::save() {
    Settings::setDownloadPath(m_pathEdit->text());
    Settings::setMaximumConcurrentTransfers(m_concurrentSpinBox->value());
    Settings::setCustomCommand(m_commandEdit->text());
    Settings::setCustomCommandEnabled(m_commandCheckBox->isChecked());
    Settings::setStartTransfersAutomatically(m_automaticCheckBox->isChecked());
    Settings::setCreateSubfolders(m_subfoldersCheckBox->isChecked());
    Settings::setClipboardMonitorEnabled(m_clipboardCheckBox->isChecked());
    Settings::setExtractArchives(m_extractCheckBox->isChecked());
    Settings::setDeleteExtractedArchives(m_deleteCheckBox->isChecked());
}

void GeneralSettingsTab::showFileDialog() {
    const QString path = QFileDialog::getExistingDirectory(this, tr("Download path"), Settings::downloadPath());

    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
    }
}

void GeneralSettingsTab::addPassword() {
    m_passwordModel->append(m_passwordEdit->text());
    m_passwordEdit->clear();
}

void GeneralSettingsTab::showPasswordContextMenu(const QPoint &pos) {
    if (!m_passwordView->currentIndex().isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Remove"));

    if (menu.exec(m_passwordView->mapToGlobal(pos))) {
        m_passwordModel->remove(m_passwordView->currentIndex().row());
    }
}

void GeneralSettingsTab::onPasswordChanged(const QString &password) {
    m_passwordButton->setEnabled(!password.isEmpty());
}
