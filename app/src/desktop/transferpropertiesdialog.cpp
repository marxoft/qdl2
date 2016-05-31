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

#include "transferpropertiesdialog.h"
#include "transferitemprioritymodel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>

TransferPropertiesDialog::TransferPropertiesDialog(TransferItem *transfer, QWidget *parent) :
    QDialog(parent),
    m_transfer(transfer),
    m_priorityModel(new TransferItemPriorityModel(this)),
    m_iconLabel(new QLabel(this)),
    m_pluginLabel(new QLabel(this)),
    m_nameLabel(new QLabel(this)),
    m_urlLabel(new QLabel(this)),
    m_speedLabel(new QLabel(this)),
    m_statusLabel(new QLabel(this)),
    m_prioritySelector(new QComboBox(this)),
    m_commandEdit(new QLineEdit(this)),
    m_commandOverrideCheckBox(new QCheckBox(tr("&Override global custom command"), this)),
    m_progressBar(new QProgressBar(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Download properties"));

    m_iconLabel->setScaledContents(true);
    m_iconLabel->setFixedSize(24, 24);

    m_pluginLabel->setWordWrap(true);
    
    m_nameLabel->setWordWrap(true);

    m_urlLabel->setTextFormat(Qt::RichText);
    m_urlLabel->setWordWrap(true);
    m_urlLabel->setOpenExternalLinks(true);

    m_speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_statusLabel->setWordWrap(true);

    m_prioritySelector->setModel(m_priorityModel);

    m_progressBar->setRange(0, 100);

    updateCustomCommand(transfer);
    updateCustomCommandOverrideEnabled(transfer);
    updateIcon(transfer);
    updateName(transfer);
    updatePluginName(transfer);
    updatePriority(transfer);
    updateProgress(transfer);
    updateSpeed(transfer);
    updateStatus(transfer);
    updateUrl(transfer);

    QWidget *container = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(container);
    hbox->addWidget(m_iconLabel);
    hbox->addWidget(m_pluginLabel);
    hbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addRow(container);
    m_layout->addRow(m_nameLabel);
    m_layout->addRow(m_urlLabel);
    m_layout->addRow(tr("&Priority:"), m_prioritySelector);
    m_layout->addRow(tr("&Custom command (%f for filename):"), m_commandEdit);
    m_layout->addRow(m_commandOverrideCheckBox);
    m_layout->addWidget(m_speedLabel);
    m_layout->addRow(m_progressBar);
    m_layout->addRow(m_statusLabel);
    m_layout->addRow(m_buttonBox);

    connect(transfer, SIGNAL(dataChanged(TransferItem*, int)), this, SLOT(onDataChanged(TransferItem*, int)));
    connect(m_prioritySelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriority(int)));
    connect(m_commandEdit, SIGNAL(editingFinished()), this, SLOT(setCustomCommand()));
    connect(m_commandOverrideCheckBox, SIGNAL(clicked(bool)), this, SLOT(setCustomCommandOverrideEnabled(bool)));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void TransferPropertiesDialog::setCustomCommand() {
    if (!m_transfer.isNull()) {
        m_transfer->setData(TransferItem::CustomCommandRole, m_commandEdit->text());
    }
}

void TransferPropertiesDialog::setCustomCommandOverrideEnabled(bool enabled) {
    if (!m_transfer.isNull()) {
        m_transfer->setData(TransferItem::CustomCommandOverrideEnabledRole, enabled);
    }
}

void TransferPropertiesDialog::setPriority(int index) {
    if (!m_transfer.isNull()) {
        m_transfer->setData(TransferItem::PriorityRole, m_prioritySelector->itemData(index));
    }
}

void TransferPropertiesDialog::updateCustomCommand(TransferItem *transfer) {
    m_commandEdit->setText(transfer->data(TransferItem::CustomCommandRole).toString());
}

void TransferPropertiesDialog::updateCustomCommandOverrideEnabled(TransferItem *transfer) {
    m_commandOverrideCheckBox->setChecked(transfer->data(TransferItem::CustomCommandOverrideEnabledRole).toBool());
}

void TransferPropertiesDialog::updateIcon(TransferItem *transfer) {
    m_iconLabel->setPixmap(QPixmap(transfer->data(TransferItem::PluginIconPathRole).toString()));
}

void TransferPropertiesDialog::updateName(TransferItem *transfer) {
    m_nameLabel->setText(transfer->data(TransferItem::NameRole).toString());
}

void TransferPropertiesDialog::updatePluginName(TransferItem *transfer) {
    m_pluginLabel->setText(transfer->data(TransferItem::PluginNameRole).toString());
}

void TransferPropertiesDialog::updatePriority(TransferItem *transfer) {
    m_prioritySelector->setCurrentIndex(qMax(0,
    m_prioritySelector->findData(transfer->data(TransferItem::PriorityRole))));
}

void TransferPropertiesDialog::updateProgress(TransferItem *transfer) {
    m_progressBar->setFormat(transfer->data(TransferItem::ProgressStringRole).toString());
    m_progressBar->setValue(transfer->data(TransferItem::ProgressRole).toInt());
}

void TransferPropertiesDialog::updateSpeed(TransferItem *transfer) {
    m_speedLabel->setText(transfer->data(TransferItem::SpeedStringRole).toString());
}

void TransferPropertiesDialog::updateStatus(TransferItem *transfer) {
    m_statusLabel->setText(transfer->data(TransferItem::StatusStringRole).toString());
}

void TransferPropertiesDialog::updateUrl(TransferItem *transfer) {
    m_urlLabel->setText(QString("<a href='%1'>%1</a>").arg(transfer->data(TransferItem::UrlRole).toString()));
}

void TransferPropertiesDialog::onDataChanged(TransferItem *transfer, int role) {
    switch (role) {
    case TransferItem::BytesTransferredRole:
    case TransferItem::ProgressRole:
    case TransferItem::SizeRole:
        updateProgress(transfer);
        break;
    case TransferItem::CustomCommandRole:
        updateCustomCommand(transfer);
        break;
    case TransferItem::CustomCommandOverrideEnabledRole:
        updateCustomCommandOverrideEnabled(transfer);
        break;
    case TransferItem::FileNameRole:
    case TransferItem::NameRole:
        updateName(transfer);
        break;
    case TransferItem::PluginIconPathRole:
        updateIcon(transfer);
        break;
    case TransferItem::PluginNameRole:
        updatePluginName(transfer);
        break;
    case TransferItem::PriorityRole:
        updatePriority(transfer);
        break;
    case TransferItem::SpeedRole:
        updateSpeed(transfer);
        break;
    case TransferItem::StatusRole:
    case TransferItem::WaitTimeRole:
        updateStatus(transfer);
        break;
    case TransferItem::UrlRole:
        updateUrl(transfer);
        break;
    default:
        break;
    }
}
