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

#include "clipboardurlsdialog.h"
#include "addurlsdialog.h"
#include "clipboardurlmodel.h"
#include "downloadrequestdialog.h"
#include "itemdelegate.h"
#include "retrieveurlsdialog.h"
#include "texteditdialog.h"
#include "transfermodel.h"
#include "urlcheckdialog.h"
#include <QDialogButtonBox>
#include <QListView>
#include <QMenu>
#include <QVBoxLayout>

static bool rowLessThan(const QModelIndex &index, const QModelIndex &other) {
    return index.row() < other.row();
}

ClipboardUrlsDialog::ClipboardUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_view(new QListView(this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Clipboard URLs"));
    setMinimumHeight(360);

    m_view->setModel(ClipboardUrlModel::instance());
    m_view->setItemDelegate(new ItemDelegate(m_view));
    m_view->setSelectionMode(QListView::MultiSelection);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setUniformItemSizes(true);

    m_layout->addWidget(m_view);

    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void ClipboardUrlsDialog::addUrls() {
    AddUrlsDialog addDialog(this);
    addDialog.setUrls(selectedUrls());

    if (addDialog.exec() == QDialog::Accepted) {
        const QStringList urls = addDialog.urls();

        if (!urls.isEmpty()) {
            if (addDialog.usePlugins()) {
                UrlCheckDialog checkDialog(this);
                checkDialog.addUrls(urls, addDialog.category(), addDialog.createSubfolder(), addDialog.priority(),
                        addDialog.customCommand(), addDialog.customCommandOverrideEnabled());
                checkDialog.exec();
            }
            else {
                TransferModel::instance()->append(urls, addDialog.requestMethod(), addDialog.requestHeaders(),
                        addDialog.postData(), addDialog.category(), addDialog.createSubfolder(), addDialog.priority(),
                        addDialog.customCommand(), addDialog.customCommandOverrideEnabled());
            }
        }
    }
}

void ClipboardUrlsDialog::removeUrls() {
    QModelIndexList rows = m_view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), rowLessThan);

    for (int i = rows.size() - 1; i >= 0; i--) {
        ClipboardUrlModel::instance()->remove(rows.at(i).row());
    }
}

void ClipboardUrlsDialog::retrieveUrls() {
    RetrieveUrlsDialog retrieveDialog(this);
    retrieveDialog.setUrls(selectedUrls());

    if (retrieveDialog.exec() == QDialog::Accepted) {
        const QStringList results = retrieveDialog.results();
        retrieveDialog.clear();

        if (!results.isEmpty()) {
            TextEditDialog dialog(results.join("\n"), this);
            dialog.setWindowTitle(tr("Retrieve URLs"));
            dialog.setLabelText(tr("Results"));
            dialog.exec();
        }
    }
}

void ClipboardUrlsDialog::fetchDownloadRequests() {
    DownloadRequestDialog dialog(this);
    dialog.addUrls(selectedUrls());
    dialog.exec();
    const QString results = dialog.resultsString();
    dialog.clear();

    if (!results.isEmpty()) {
        TextEditDialog dialog(results, this);
        dialog.setWindowTitle(tr("Retrieve download requests"));
        dialog.setLabelText(tr("Results"));
        dialog.exec();
    }
}

QStringList ClipboardUrlsDialog::selectedUrls() const {
    if (!m_view->selectionModel()->hasSelection()) {
        return QStringList();
    }
    
    QStringList list;
    QModelIndexList rows = m_view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), rowLessThan);

    for (int i = 0; i < rows.size(); i++) {
        list << rows.at(i).data().toString();
    }

    return list;
}

void ClipboardUrlsDialog::showContextMenu(const QPoint &pos) {
    if (!m_view->selectionModel()->hasSelection()) {
        return;
    }

    QMenu menu(this);
    QAction *addAction = menu.addAction(tr("Add URL"));
    QAction *retrieveAction = menu.addAction(tr("Retrieve URLs"));
    QAction *downloadAction = menu.addAction(tr("Retrieve download requests"));
    QAction *removeAction = menu.addAction(tr("Remove"));
    QAction *action = menu.exec(m_view->mapToGlobal(pos));
    
    if (!action) {
        return;
    }
    
    if (action == addAction) {
        addUrls();
    }
    else if (action == retrieveAction) {
        retrieveUrls();
    }
    else if (action == downloadAction) {
        fetchDownloadRequests();
    }
    else if (action == removeAction) {
        removeUrls();
    }
}
