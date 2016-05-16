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

    if (UrlCheckModel::instance()->status() != UrlCheckModel::Active) {
        UrlCheckModel::instance()->clear();
    }

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
    connect(UrlCheckModel::instance(), SIGNAL(statusChanged(UrlCheckModel::Status)),
            this, SLOT(onStatusChanged(UrlCheckModel::Status)));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    onStatusChanged(UrlCheckModel::instance()->status());
}

void UrlCheckDialog::addUrl(const QString &url, const QString &pluginId) {
    UrlCheckModel::instance()->append(url, pluginId);
}

void UrlCheckDialog::addUrls(const QStringList &urls, const QString &pluginId) {
    UrlCheckModel::instance()->append(urls, pluginId);
}

void UrlCheckDialog::reject() {
    UrlCheckModel::instance()->clear();
    QDialog::reject();
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

void UrlCheckDialog::onStatusChanged(UrlCheckModel::Status status) {
    m_statusLabel->setText(UrlCheckModel::instance()->statusString());

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
