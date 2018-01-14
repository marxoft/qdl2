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

#include "downloadrequestdialog.h"
#include "captchadialog.h"
#include "captchatype.h"
#include "nocaptchadialog.h"
#include "pluginsettingsdialog.h"
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

DownloadRequestDialog::DownloadRequestDialog(QWidget *parent) :
    QDialog(parent),
    m_view(new QTreeView(this)),
    m_progressBar(new QProgressBar(this)),
    m_statusLabel(new QLabel(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Download requests"));

    m_view->setModel(DownloadRequestModel::instance());
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setItemsExpandable(false);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setRootIsDecorated(false);
    m_view->header()->setStretchLastSection(false);
    m_view->header()->resizeSection(1, 32);
#if QT_VERSION >= 0x050000
    m_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    m_view->header()->setResizeMode(0, QHeaderView::Stretch);
#endif

    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(DownloadRequestModel::instance()->progress());

    m_layout->addWidget(m_view);
    m_layout->addWidget(m_progressBar);
    m_layout->addWidget(m_statusLabel);
    m_layout->addWidget(m_buttonBox);

    connect(DownloadRequestModel::instance(), SIGNAL(progressChanged(int)), m_progressBar, SLOT(setValue(int)));
    connect(DownloadRequestModel::instance(), SIGNAL(captchaRequest(int, QByteArray)),
            this, SLOT(showCaptchaDialog(int, QByteArray)));
    connect(DownloadRequestModel::instance(), SIGNAL(captchaTimeoutChanged(int)), this, SLOT(updateStatusLabel()));
    connect(DownloadRequestModel::instance(), SIGNAL(requestedSettingsTimeoutChanged(int)),
            this, SLOT(updateStatusLabel()));
    connect(DownloadRequestModel::instance(), SIGNAL(settingsRequest(QString, QVariantList)),
            this, SLOT(showPluginSettingsDialog(QString, QVariantList)));
    connect(DownloadRequestModel::instance(), SIGNAL(statusChanged(DownloadRequestModel::Status)),
            this, SLOT(onStatusChanged(DownloadRequestModel::Status)));
    connect(DownloadRequestModel::instance(), SIGNAL(waitTimeChanged(int)), this, SLOT(updateStatusLabel()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    onStatusChanged(DownloadRequestModel::instance()->status());
}

void DownloadRequestDialog::addUrl(const QString &url) {
    DownloadRequestModel::instance()->append(url);
}

void DownloadRequestDialog::addUrls(const QStringList &urls) {
    DownloadRequestModel::instance()->append(urls);
}

void DownloadRequestDialog::clear() {
    DownloadRequestModel::instance()->clear();
}

QString DownloadRequestDialog::resultsString() const {
    return DownloadRequestModel::instance()->resultsString();
}

void DownloadRequestDialog::showCaptchaDialog(int captchaType, const QByteArray &captchaData) {
    if (captchaType == CaptchaType::NoCaptcha) {
        NoCaptchaDialog dialog(this);
        dialog.setHtml(QString::fromUtf8(captchaData), m_view->currentIndex().data(DownloadRequestModel::UrlRole)
                .toString());
        dialog.setTimeout(DownloadRequestModel::instance()->captchaTimeout());
        connect(DownloadRequestModel::instance(), SIGNAL(statusChanged(DownloadRequestModel::Status)),
                &dialog, SLOT(close()));

        switch (dialog.exec()) {
        case QDialog::Accepted:
            DownloadRequestModel::instance()->submitCaptchaResponse(dialog.response());
            break;
        case QDialog::Rejected:
            DownloadRequestModel::instance()->submitCaptchaResponse(QString());
            break;
        default:
            break;
        }
    }
    else {
        CaptchaDialog dialog(this);
        dialog.setImage(QImage::fromData(QByteArray::fromBase64(captchaData)));
        dialog.setTimeout(DownloadRequestModel::instance()->captchaTimeout());
        connect(DownloadRequestModel::instance(), SIGNAL(statusChanged(DownloadRequestModel::Status)), &dialog, SLOT(close()));

        switch (dialog.exec()) {
        case QDialog::Accepted:
            DownloadRequestModel::instance()->submitCaptchaResponse(dialog.response());
            break;
        case QDialog::Rejected:
            DownloadRequestModel::instance()->submitCaptchaResponse(QString());
            break;
        default:
            break;
        }
    }
}

void DownloadRequestDialog::showPluginSettingsDialog(const QString &title, const QVariantList &settings) {
    PluginSettingsDialog dialog(settings, this);
    dialog.setWindowTitle(title);
    dialog.setTimeout(DownloadRequestModel::instance()->requestedSettingsTimeout());
    connect(DownloadRequestModel::instance(), SIGNAL(statusChanged(DownloadRequestModel::Status)),
            &dialog, SLOT(close()));
    
    switch (dialog.exec()) {
    case QDialog::Accepted:
        DownloadRequestModel::instance()->submitSettingsResponse(dialog.settings());
        break;
    case QDialog::Rejected:
        DownloadRequestModel::instance()->submitSettingsResponse(QVariantMap());
        break;
    default:
        break;
    }
}

void DownloadRequestDialog::updateStatusLabel() {
    m_statusLabel->setText(DownloadRequestModel::instance()->statusString());
}

void DownloadRequestDialog::onStatusChanged(DownloadRequestModel::Status status) {
    updateStatusLabel();

    switch (status) {
    case DownloadRequestModel::Completed:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        break;
    default:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    }
}
