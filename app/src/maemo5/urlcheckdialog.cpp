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

#include "urlcheckdialog.h"
#include "captchadialog.h"
#include "captchatype.h"
#include "nocaptchadialog.h"
#include "pluginsettingsdialog.h"
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeView>

UrlCheckDialog::UrlCheckDialog(QWidget *parent) :
    QDialog(parent),
    m_view(new QTreeView(this)),
    m_progressBar(new QProgressBar(this)),
    m_statusLabel(new QLabel(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("Check URLs"));
    setMinimumHeight(360);

    m_view->setModel(UrlCheckModel::instance());
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setItemsExpandable(false);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setRootIsDecorated(false);
    m_view->header()->setStretchLastSection(false);
    m_view->header()->resizeSection(1, 48);
#if QT_VERSION >= 0x050000
    m_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    m_view->header()->setResizeMode(0, QHeaderView::Stretch);
#endif

    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(UrlCheckModel::instance()->progress());

    m_layout->addWidget(m_view, 0, 0);
    m_layout->addWidget(m_progressBar, 1, 0);
    m_layout->addWidget(m_statusLabel, 2, 0);
    m_layout->addWidget(m_buttonBox, 2, 1);

    connect(UrlCheckModel::instance(), SIGNAL(progressChanged(int)), m_progressBar, SLOT(setValue(int)));
    connect(UrlCheckModel::instance(), SIGNAL(captchaRequest(int, QByteArray)),
            this, SLOT(showCaptchaDialog(int, QByteArray)));
    connect(UrlCheckModel::instance(), SIGNAL(captchaTimeoutChanged(int)), this, SLOT(updateStatusLabel()));
    connect(UrlCheckModel::instance(), SIGNAL(requestedSettingsTimeoutChanged(int)),
            this, SLOT(updateStatusLabel()));
    connect(UrlCheckModel::instance(), SIGNAL(settingsRequest(QString, QVariantList)),
            this, SLOT(showPluginSettingsDialog(QString, QVariantList)));
    connect(UrlCheckModel::instance(), SIGNAL(statusChanged(UrlCheckModel::Status)),
            this, SLOT(onStatusChanged(UrlCheckModel::Status)));
    connect(UrlCheckModel::instance(), SIGNAL(waitTimeChanged(int)), this, SLOT(updateStatusLabel()));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    onStatusChanged(UrlCheckModel::instance()->status());
}

void UrlCheckDialog::addUrl(const QString &url, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand) {
    UrlCheckModel::instance()->append(url, category, createSubfolder, priority, customCommand, overrideGlobalCommand);
}

void UrlCheckDialog::addUrls(const QStringList &urls, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand) {
    UrlCheckModel::instance()->append(urls, category, createSubfolder, priority, customCommand, overrideGlobalCommand);
}

void UrlCheckDialog::accept() {
    clear();
    QDialog::accept();
}

void UrlCheckDialog::reject() {
    clear();
    QDialog::reject();
}

void UrlCheckDialog::clear() {
    UrlCheckModel::instance()->clear();
}

void UrlCheckDialog::showContextMenu(const QPoint &pos) {
    if (!m_view->currentIndex().isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction(tr("Remove"));

    if (menu.exec(m_view->mapToGlobal(pos))) {
        UrlCheckModel::instance()->remove(m_view->currentIndex().row());
    }
}

void UrlCheckDialog::showCaptchaDialog(int captchaType, const QByteArray &captchaData) {
    if (captchaType == CaptchaType::NoCaptcha) {
        NoCaptchaDialog dialog(this);
        dialog.setHtml(QString::fromUtf8(captchaData), m_view->currentIndex().data(UrlCheckModel::UrlRole).toString());
        dialog.setTimeout(UrlCheckModel::instance()->captchaTimeout());
        connect(UrlCheckModel::instance(), SIGNAL(statusChanged(UrlCheckModel::Status)), &dialog, SLOT(close()));

        switch (dialog.exec()) {
        case QDialog::Accepted:
            UrlCheckModel::instance()->submitCaptchaResponse(dialog.response());
            break;
        case QDialog::Rejected:
            UrlCheckModel::instance()->submitCaptchaResponse(QString());
            break;
        default:
            break;
        }
    }
    else {
        CaptchaDialog dialog(this);
        dialog.setImage(QImage::fromData(QByteArray::fromBase64(captchaData)));
        dialog.setTimeout(UrlCheckModel::instance()->captchaTimeout());
        connect(UrlCheckModel::instance(), SIGNAL(statusChanged(UrlCheckModel::Status)), &dialog, SLOT(close()));

        switch (dialog.exec()) {
        case QDialog::Accepted:
            UrlCheckModel::instance()->submitCaptchaResponse(dialog.response());
            break;
        case QDialog::Rejected:
            UrlCheckModel::instance()->submitCaptchaResponse(QString());
            break;
        default:
            break;
        }
    }
}

void UrlCheckDialog::showPluginSettingsDialog(const QString &title, const QVariantList &settings) {
    PluginSettingsDialog dialog(settings, this);
    dialog.setWindowTitle(title);
    dialog.setTimeout(UrlCheckModel::instance()->requestedSettingsTimeout());
    connect(UrlCheckModel::instance(), SIGNAL(statusChanged(UrlCheckModel::Status)), &dialog, SLOT(close()));
    
    switch (dialog.exec()) {
    case QDialog::Accepted:
        UrlCheckModel::instance()->submitSettingsResponse(dialog.settings());
        break;
    case QDialog::Rejected:
        UrlCheckModel::instance()->submitSettingsResponse(QVariantMap());
        break;
    default:
        break;
    }
}

void UrlCheckDialog::updateStatusLabel() {
    m_statusLabel->setText(UrlCheckModel::instance()->statusString());
}

void UrlCheckDialog::onStatusChanged(UrlCheckModel::Status status) {
    updateStatusLabel();

    switch (status) {
    case UrlCheckModel::Completed:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        break;
    default:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    }
}
