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

#include "packagepropertiesdialog.h"
#include "categoryselectionmodel.h"
#include "transferitemprioritymodel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QProgressBar>

PackagePropertiesDialog::PackagePropertiesDialog(TransferItem *package, QWidget *parent) :
    QDialog(parent),
    m_package(package),
    m_categoryModel(new CategorySelectionModel(this)),
    m_priorityModel(new TransferItemPriorityModel(this)),
    m_nameLabel(new QLabel(this)),
    m_statusLabel(new QLabel(this)),
    m_subfolderCheckBox(new QCheckBox(tr("Create sub&folder"), this)),
    m_categorySelector(new QComboBox(this)),
    m_prioritySelector(new QComboBox(this)),
    m_progressBar(new QProgressBar(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Package properties"));

    m_nameLabel->setWordWrap(true);

    m_statusLabel->setWordWrap(true);

    m_categorySelector->setModel(m_categoryModel);
    
    m_prioritySelector->setModel(m_priorityModel);

    m_progressBar->setRange(0, 100);

    updateCategory(package);
    updateCreateSubfolder(package);
    updateName(package);
    updatePriority(package);
    updateProgress(package);
    updateStatus(package);

    m_layout->addRow(m_nameLabel);
    m_layout->addRow(m_subfolderCheckBox);
    m_layout->addRow(tr("&Category:"), m_categorySelector);
    m_layout->addRow(tr("&Priority:"), m_prioritySelector);
    m_layout->addRow(m_progressBar);
    m_layout->addRow(m_statusLabel);
    m_layout->addRow(m_buttonBox);

    connect(package, SIGNAL(dataChanged(TransferItem*, int)), this, SLOT(onDataChanged(TransferItem*, int)));
    connect(m_subfolderCheckBox, SIGNAL(clicked(bool)), this, SLOT(setCreateSubfolder(bool)));
    connect(m_categorySelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setCategory(int)));
    connect(m_prioritySelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriority(int)));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void PackagePropertiesDialog::setCategory(int index) {
    if (!m_package.isNull()) {
        m_package->setData(TransferItem::CategoryRole, m_categorySelector->itemData(index));
        m_package->save();
    }
}

void PackagePropertiesDialog::setCreateSubfolder(bool enabled) {
    if (!m_package.isNull()) {
        m_package->setData(TransferItem::CreateSubfolderRole, enabled);
        m_package->save();
    }
}

void PackagePropertiesDialog::setPriority(int index) {
    if (!m_package.isNull()) {
        m_package->setData(TransferItem::PriorityRole, m_prioritySelector->itemData(index));
        m_package->save();
    }
}

void PackagePropertiesDialog::updateCategory(TransferItem *package) {
    m_categorySelector->setCurrentIndex(qMax(0,
    m_categorySelector->findData(package->data(TransferItem::CategoryRole))));
}

void PackagePropertiesDialog::updateCreateSubfolder(TransferItem *package) {
    m_subfolderCheckBox->setChecked(package->data(TransferItem::CreateSubfolderRole).toBool());
}

void PackagePropertiesDialog::updateName(TransferItem *package) {
    m_nameLabel->setText(package->data(TransferItem::NameRole).toString());
}

void PackagePropertiesDialog::updatePriority(TransferItem *package) {
    m_prioritySelector->setCurrentIndex(qMax(0,
    m_prioritySelector->findData(package->data(TransferItem::PriorityRole))));
}

void PackagePropertiesDialog::updateProgress(TransferItem *package) {
    m_progressBar->setFormat(package->data(TransferItem::ProgressStringRole).toString());
    m_progressBar->setValue(package->data(TransferItem::ProgressRole).toInt());
}

void PackagePropertiesDialog::updateStatus(TransferItem *package) {
    m_statusLabel->setText(package->data(TransferItem::StatusStringRole).toString());
}

void PackagePropertiesDialog::onDataChanged(TransferItem *package, int role) {
    switch (role) {
    case TransferItem::CategoryRole:
        updateCategory(package);
        break;
    case TransferItem::CreateSubfolderRole:
        updateCreateSubfolder(package);
        break;
    case TransferItem::NameRole:
        updateName(package);
        break;
    case TransferItem::PriorityRole:
        updatePriority(package);
        break;
    case TransferItem::ProgressRole:
    case TransferItem::RowCountRole:
        updateProgress(package);
        break;
    case TransferItem::StatusRole:
        updateStatus(package);
        break;
    default:
        break;
    }
}
