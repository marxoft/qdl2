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
#include "downloadrequestdialog.h"
#include "itemdelegate.h"
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
#include <QMaemo5Style>
#include <QMenu>
#include <QSplitter>
#include <QWebHitTestResult>
#include <QWebView>

const QByteArray SearchPage::STYLE_SHEET = QByteArray("data:text/css;charset=utf-8;base64,")
    + QString("html { background-color: %1; color: %2; font-family: %3; font-size: 18pt; } img { max-width: 100%; } iframe { max-width: 100%; }")
    .arg(QMaemo5Style::standardColor("DefaultBackgroundColor").name()).arg(QMaemo5Style::standardColor("DefaultTextColor").name())
    .arg(QFont().family()).toUtf8().toBase64();

SearchPage::SearchPage(QWidget *parent) :
    Page(parent),
    m_model(new SearchModel(this)),
    m_splitter(new QSplitter(Qt::Horizontal, this)),
    m_view(new QListView(m_splitter)),
    m_browser(new QWebView(m_splitter)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Search"));
    
    m_splitter->addWidget(m_view);
    m_splitter->addWidget(m_browser);
    
    m_view->setModel(m_model);
    m_view->setItemDelegate(new ItemDelegate(m_view));
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setUniformItemSizes(true);

    m_browser->setContextMenuPolicy(Qt::CustomContextMenu);
    m_browser->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_browser->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    m_browser->settings()->setUserStyleSheetUrl(QUrl::fromEncoded(STYLE_SHEET));
    
    m_layout->addWidget(m_splitter);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    m_splitter->restoreState(Settings::searchPageState());
    
    connect(m_model, SIGNAL(settingsRequest(QString, QVariantList)),
            this, SLOT(showPluginSettingsDialog(QString, QVariantList)));
    connect(m_model, SIGNAL(statusChanged(SearchModel::Status)), this, SLOT(onModelStatusChanged()));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(showItemDetails(QModelIndex)));
    connect(m_browser, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showBrowserContextMenu(QPoint)));
}

SearchPage::~SearchPage() {
    QWebSettings::clearMemoryCaches();
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

void SearchPage::addUrl(const QString &url) {
    AddUrlsDialog addDialog(this);
    addDialog.setText(url);

    if (addDialog.exec() == QDialog::Accepted) {
        const QStringList urls = addDialog.urls();

        if (!urls.isEmpty()) {
            if (addDialog.usePlugins()) {
                UrlCheckDialog checkDialog(this);
                checkDialog.addUrls(urls, addDialog.category(), addDialog.createSubfolder(), addDialog.priority(),
                        addDialog.customCommand(), addDialog.customCommandOverrideEnabled(),
                        addDialog.startAutomatically());
                checkDialog.exec();
            }
            else {
                TransferModel::instance()->append(urls, addDialog.requestMethod(), addDialog.requestHeaders(),
                        addDialog.postData(), addDialog.category(), addDialog.createSubfolder(), addDialog.priority(),
                        addDialog.customCommand(), addDialog.customCommandOverrideEnabled(),
                        addDialog.startAutomatically());
            }
        }
    }
}

void SearchPage::copyUrl(const QString &url) {
    QApplication::clipboard()->setText(url);
}

void SearchPage::retrieveUrls(const QString &url) {
    RetrieveUrlsDialog retrieveDialog(this);
    retrieveDialog.setText(url);

    if (retrieveDialog.exec() == QDialog::Accepted) {
        const QStringList results = retrieveDialog.results();
        retrieveDialog.clear();

        if (!results.isEmpty()) {
            TextEditDialog dialog(results.join("\n"), this);
            dialog.setWindowTitle(tr("Retrieve URLs"));
            dialog.setLabelText(tr("Results:"));
            dialog.exec();
        }
    }
}

void SearchPage::fetchDownloadRequests(const QString &url) {
    DownloadRequestDialog dialog(this);
    dialog.addUrl(url);
    dialog.exec();
    const QString results = dialog.resultsString();
    dialog.clear();

    if (!results.isEmpty()) {
        TextEditDialog dialog(results, this);
        dialog.setWindowTitle(tr("Retrieve download requests"));
        dialog.setLabelText(tr("Results:"));
        dialog.exec();
    }
}

void SearchPage::showContextMenu(const QPoint &pos) {
    const QModelIndex index = m_view->currentIndex();
    
    if (!index.isValid()) {
        return;
    }
    
    QMenu menu(this);
    QAction *copyAction = menu.addAction(QIcon::fromTheme("edit-copy"), tr("&Copy URL"));
    QAction *addAction = menu.addAction(QIcon::fromTheme("list-add"), tr("&Add URL"));
    QAction *retrieveAction = menu.addAction(QIcon::fromTheme("folder-remote"), tr("&Retrieve URLs"));
    QAction *downloadAction = menu.addAction(QIcon::fromTheme("download"), tr("Retrieve &download requests"));
    QAction *action = menu.exec(m_view->mapToGlobal(pos));
    
    if (!action) {
        return;
    }
    
    if (action == copyAction) {
        copyUrl(index.data(SearchModel::UrlRole).toString());
    }
    else if (action == addAction) {
        addUrl(index.data(SearchModel::UrlRole).toString());
    }
    else if (action == retrieveAction) {
        retrieveUrls(index.data(SearchModel::UrlRole).toString());
    }
    else if (action == downloadAction) {
        fetchDownloadRequests(index.data(SearchModel::UrlRole).toString());
    }
}

void SearchPage::showBrowserContextMenu(const QPoint &pos) {
    const QWebHitTestResult result = m_browser->page()->mainFrame()->hitTestContent(pos);
    const QString url = result.linkUrl().toString();

    if (url.isEmpty()) {
        return;
    }

    QMenu menu(this);
    QAction *copyAction = menu.addAction(QIcon::fromTheme("edit-copy"), tr("&Copy URL"));
    QAction *addAction = menu.addAction(QIcon::fromTheme("list-add"), tr("&Add URL"));
    QAction *retrieveAction = menu.addAction(QIcon::fromTheme("folder-remote"), tr("&Retrieve URLs"));
    QAction *downloadAction = menu.addAction(QIcon::fromTheme("download"), tr("Retrieve &download requests"));
    QAction *action = menu.exec(m_browser->mapToGlobal(pos));
    
    if (!action) {
        return;
    }
    
    if (action == copyAction) {
        copyUrl(url);
    }
    else if (action == addAction) {
        addUrl(url);
    }
    else if (action == retrieveAction) {
        retrieveUrls(url);
    }
    else if (action == downloadAction) {
        fetchDownloadRequests(url);
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
