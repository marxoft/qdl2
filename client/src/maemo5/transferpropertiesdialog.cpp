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

#include "transferpropertiesdialog.h"
#include "imagecache.h"
#include "transferitemprioritymodel.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QScrollArea>
#include <QVBoxLayout>

TransferPropertiesDialog::TransferPropertiesDialog(TransferItem *transfer, QWidget *parent) :
    QDialog(parent),
    m_transfer(transfer),
    m_priorityModel(new TransferItemPriorityModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_iconLabel(new QLabel(m_container)),
    m_pluginLabel(new QLabel(m_container)),
    m_nameLabel(new QLabel(m_container)),
    m_urlLabel(new QLabel(m_container)),
    m_speedLabel(new QLabel(m_container)),
    m_statusLabel(new QLabel(m_container)),
    m_prioritySelector(new ValueSelector(tr("Priority"), m_container)),
    m_commandEdit(new QLineEdit(m_container)),
    m_commandOverrideCheckBox(new QCheckBox(tr("&Override global custom command"), m_container)),
    m_progressBar(new QProgressBar(m_container)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Download properties"));
    setMinimumHeight(360);

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);

    m_iconLabel->setScaledContents(true);
    m_iconLabel->setFixedSize(48, 48);

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

    m_vbox->addWidget(container);
    m_vbox->addWidget(m_nameLabel);
    m_vbox->addWidget(m_urlLabel);
    m_vbox->addWidget(m_prioritySelector);
    m_vbox->addWidget(new QLabel(tr("Custom command (%f for filename)"), m_container));
    m_vbox->addWidget(m_commandEdit);
    m_vbox->addWidget(m_commandOverrideCheckBox);
    m_vbox->addWidget(m_speedLabel);
    m_vbox->addWidget(m_progressBar);
    m_vbox->addWidget(m_statusLabel);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);

    connect(transfer, SIGNAL(dataChanged(TransferItem*, int)), this, SLOT(onDataChanged(TransferItem*, int)));
    connect(m_prioritySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setPriority(QVariant)));
    connect(m_commandEdit, SIGNAL(editingFinished()), this, SLOT(setCustomCommand()));
    connect(m_commandOverrideCheckBox, SIGNAL(clicked(bool)), this, SLOT(setCustomCommandOverrideEnabled(bool)));
}

void TransferPropertiesDialog::setCustomCommand() {
    if (!m_transfer.isNull()) {
        m_transfer->setData(TransferItem::CustomCommandRole, m_commandEdit->text());
        m_transfer->save();
    }
}

void TransferPropertiesDialog::setCustomCommandOverrideEnabled(bool enabled) {
    if (!m_transfer.isNull()) {
        m_transfer->setData(TransferItem::CustomCommandOverrideEnabledRole, enabled);
        m_transfer->save();
    }
}

void TransferPropertiesDialog::setPriority(const QVariant &priority) {
    if (!m_transfer.isNull()) {
        m_transfer->setData(TransferItem::PriorityRole, priority);
        m_transfer->save();
    }
}

void TransferPropertiesDialog::updateCustomCommand(TransferItem *transfer) {
    m_commandEdit->setText(transfer->data(TransferItem::CustomCommandRole).toString());
}

void TransferPropertiesDialog::updateCustomCommandOverrideEnabled(TransferItem *transfer) {
    m_commandOverrideCheckBox->setChecked(transfer->data(TransferItem::CustomCommandOverrideEnabledRole).toBool());
}

void TransferPropertiesDialog::updateIcon(TransferItem *transfer) {
    m_iconLabel->setPixmap(ImageCache::instance()->image(transfer->data(TransferItem::PluginIconPathRole).toString()));
}

void TransferPropertiesDialog::updateName(TransferItem *transfer) {
    m_nameLabel->setText(transfer->data(TransferItem::NameRole).toString());
}

void TransferPropertiesDialog::updatePluginName(TransferItem *transfer) {
    m_pluginLabel->setText(transfer->data(TransferItem::PluginNameRole).toString());
}

void TransferPropertiesDialog::updatePriority(TransferItem *transfer) {
    m_prioritySelector->setValue(transfer->data(TransferItem::PriorityRole));
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
    case TransferItem::PluginIconRole:
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
