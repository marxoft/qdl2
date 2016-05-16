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

#include "packagepropertiesdialog.h"
#include "categoryselectionmodel.h"
#include "transferitemprioritymodel.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QVBoxLayout>

PackagePropertiesDialog::PackagePropertiesDialog(TransferItem *package, QWidget *parent) :
    QDialog(parent),
    m_package(package),
    m_categoryModel(new CategorySelectionModel(this)),
    m_priorityModel(new TransferItemPriorityModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_nameLabel(new QLabel(m_container)),
    m_statusLabel(new QLabel(m_container)),
    m_subfolderCheckBox(new QCheckBox(tr("Create subfolder"), m_container)),
    m_categorySelector(new ValueSelector(tr("Category"), m_container)),
    m_prioritySelector(new ValueSelector(tr("Priority"), m_container)),
    m_progressBar(new QProgressBar(m_container)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Package properties"));

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);

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

    m_vbox->addWidget(m_nameLabel);
    m_vbox->addWidget(m_subfolderCheckBox);
    m_vbox->addWidget(m_categorySelector);
    m_vbox->addWidget(m_prioritySelector);
    m_vbox->addWidget(m_progressBar);
    m_vbox->addWidget(m_statusLabel);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);

    connect(package, SIGNAL(dataChanged(TransferItem*, int)), this, SLOT(onDataChanged(TransferItem*, int)));
    connect(m_subfolderCheckBox, SIGNAL(clicked(bool)), this, SLOT(setCreateSubfolder(bool)));
    connect(m_categorySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setCategory(QVariant)));
    connect(m_prioritySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setPriority(QVariant)));
}

void PackagePropertiesDialog::setCategory(const QVariant &category) {
    if (!m_package.isNull()) {
        m_package->setData(TransferItem::CategoryRole, category);
    }
}

void PackagePropertiesDialog::setCreateSubfolder(bool enabled) {
    if (!m_package.isNull()) {
        m_package->setData(TransferItem::CreateSubfolderRole, enabled);
    }
}

void PackagePropertiesDialog::setPriority(const QVariant &priority) {
    if (!m_package.isNull()) {
        m_package->setData(TransferItem::PriorityRole, priority);
    }
}

void PackagePropertiesDialog::updateCategory(TransferItem *package) {
    m_categorySelector->setValue(package->data(TransferItem::CategoryRole));
}

void PackagePropertiesDialog::updateCreateSubfolder(TransferItem *package) {
    m_subfolderCheckBox->setChecked(package->data(TransferItem::CreateSubfolderRole).toBool());
}

void PackagePropertiesDialog::updateName(TransferItem *package) {
    m_nameLabel->setText(package->data(TransferItem::NameRole).toString());
}

void PackagePropertiesDialog::updatePriority(TransferItem *package) {
    m_prioritySelector->setValue(package->data(TransferItem::PriorityRole));
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
