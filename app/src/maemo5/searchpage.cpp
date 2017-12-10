/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "searchpage.h"
#include "addurlsdialog.h"
#include "browser.h"
#include "downloadrequestdialog.h"
#include "pluginsettingsdialog.h"
#include "retrieveurlsdialog.h"
#include "settings.h"
#include "texteditdialog.h"
#include "transfermodel.h"
#include "urlcheckdialog.h"
#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QListView>
#include <QMenu>
#include <QSplitter>

SearchPage::SearchPage(QWidget *parent) :
    Page(parent),
    m_model(new SearchModel(this)),
    m_splitter(new QSplitter(Qt::Horizontal, this)),
    m_view(new QListView(m_splitter)),
    m_browser(new Browser(m_splitter)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Search"));
    
    m_splitter->addWidget(m_view);
    m_splitter->addWidget(m_browser);
    
    m_view->setModel(m_model);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setUniformItemSizes(true);
    
    m_layout->addWidget(m_splitter);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    m_splitter->restoreState(Settings::searchPageState());
    
    connect(m_model, SIGNAL(settingsRequest(QString, QVariantList)),
            this, SLOT(showPluginSettingsDialog(QString, QVariantList)));
    connect(m_model, SIGNAL(statusChanged(SearchModel::Status)), this, SLOT(onModelStatusChanged()));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(showItemDetails(QModelIndex)));
}

QString SearchPage::errorString() const {
    return m_model->errorString();
}

Page::Status SearchPage::status() const {
    switch (m_model->status()) {
    case SearchModel::Active:
    case SearchModel::AwaitingSettingsResponse:
        return Active;
    case SearchModel::Completed:
        return Ready;
    case SearchModel::Canceled:
        return Canceled;
    case SearchModel::Error:
        return Error;
    default:
        return Idle;
    }
}

QString SearchPage::statusString() const {
    switch (status()) {
    case Ready:
        return tr("%1 results").arg(m_model->rowCount());
    default:
        return m_model->statusString();
    }
}

void SearchPage::closeEvent(QCloseEvent *event) {
    Settings::setSearchPageState(m_splitter->saveState());
    Page::closeEvent(event);
}

void SearchPage::search(const QString &pluginId) {
    m_model->search(pluginId);
}

void SearchPage::addUrl(const QModelIndex &index) {
    AddUrlsDialog addDialog(this);
    addDialog.setText(index.data(SearchModel::UrlRole).toString());

    if (addDialog.exec() == QDialog::Accepted) {
        const QStringList urls = addDialog.urls();

        if (!urls.isEmpty()) {
            if (addDialog.usePlugins()) {
                UrlCheckDialog checkDialog(this);
                checkDialog.addUrls(urls);
                checkDialog.exec();
            }
            else {
                TransferModel::instance()->append(urls, addDialog.requestMethod(), addDialog.requestHeaders(),
                                                  addDialog.postData());
            }
        }
    }
}

void SearchPage::copyUrl(const QModelIndex &index) {
    QApplication::clipboard()->setText(index.data(SearchModel::UrlRole).toString());
}

void SearchPage::retrieveUrls(const QModelIndex &index) {
    RetrieveUrlsDialog retrieveDialog(this);
    retrieveDialog.setText(index.data(SearchModel::UrlRole).toString());

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

void SearchPage::fetchDownloadRequests(const QModelIndex &index) {
    DownloadRequestDialog dialog(this);
    dialog.addUrl(index.data(SearchModel::UrlRole).toString());
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

void SearchPage::showContextMenu(const QPoint &pos) {
    const QModelIndex index = m_view->currentIndex();
    
    if (!index.isValid()) {
        return;
    }
    
    QMenu menu(this);
    QAction *copyAction = menu.addAction(tr("Copy URL"));
    QAction *addAction = menu.addAction(tr("Add URL"));
    QAction *retrieveAction = menu.addAction(tr("Retrieve URLs"));
    QAction *downloadAction = menu.addAction(tr("Retrieve download requests"));
    QAction *action = menu.exec(m_view->mapToGlobal(pos));
    
    if (!action) {
        return;
    }
    
    if (action == copyAction) {
        copyUrl(index);
    }
    else if (action == addAction) {
        addUrl(index);
    }
    else if (action == retrieveAction) {
        retrieveUrls(index);
    }
    else if (action == downloadAction) {
        fetchDownloadRequests(index);
    }
}

void SearchPage::showItemDetails(const QModelIndex &index) {
    m_browser->setHtml(QString("<p style='font-weight: bold'>%1</p><p>%2</p>")
                       .arg(index.data(SearchModel::NameRole).toString())
                       .arg(index.data(SearchModel::DescriptionRole).toString()));
}

void SearchPage::showPluginSettingsDialog(const QString &title, const QVariantList &settings) {
    PluginSettingsDialog dialog(settings, this);
    dialog.setWindowTitle(title);
    dialog.setTimeout(-1);
    
    switch (dialog.exec()) {
    case QDialog::Accepted:
        m_model->submitSettingsResponse(dialog.settings());
        break;
    case QDialog::Rejected:
        m_model->submitSettingsResponse(QVariantMap());
        break;
    default:
        break;
    }
}

void SearchPage::onModelStatusChanged() {
    emit statusChanged(status());
}
