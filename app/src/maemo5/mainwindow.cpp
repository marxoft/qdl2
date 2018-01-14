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

#include "mainwindow.h"
#include "aboutdialog.h"
#include "actionmodel.h"
#include "addurlsdialog.h"
#include "captchadialog.h"
#include "captchatype.h"
#include "clipboardurlsdialog.h"
#include "concurrenttransfersmodel.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "downloadrequestdialog.h"
#include "nocaptchadialog.h"
#include "packagepropertiesdialog.h"
#include "pluginsettingsdialog.h"
#include "qdl.h"
#include "recaptchapluginmanager.h"
#include "retrieveurlsdialog.h"
#include "searchdialog.h"
#include "searchpage.h"
#include "searchpluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "settingsdialog.h"
#include "texteditdialog.h"
#include "transfermodel.h"
#include "transferpropertiesdialog.h"
#include "urlcheckdialog.h"
#include "utils.h"
#include "valueselectoraction.h"
#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_transferMenu(new QMenu(tr("Download"), this)),
    m_packageMenu(new QMenu(tr("Package"), this)),
    m_concurrentAction(new ValueSelectorAction(tr("Maximum concurrent DLs"), this)),
    m_nextAction(new ValueSelectorAction(tr("After current DLs"), this)),
    m_searchAction(new QAction(tr("Search"), this)),
    m_settingsAction(new QAction(tr("Settings"), this)),
    m_pluginsAction(new QAction(tr("Load plugins"), this)),
    m_aboutAction(new QAction(tr("About"), this)),
    m_closePageAction(new QAction(tr("Close tab"), this)),
    m_firstPageAction(new QAction(tr("First tab"), this)),
    m_lastPageAction(new QAction(tr("Last tab"), this)),
    m_nextPageAction(new QAction(tr("Next tab"), this)),
    m_previousPageAction(new QAction(tr("Previous tab"), this)),
    m_scrollBottomAction(new QAction(tr("Scroll to bottom"), this)),
    m_scrollTopAction(new QAction(tr("Scroll to top"), this)),
    m_addUrlsAction(new QAction(QIcon::fromTheme("general_add"), tr("Add URLs"), this)),
    m_importUrlsAction(new QAction(QIcon::fromTheme("general_toolbar_folder"), tr("Import URLs"), this)),
    m_retrieveUrlsAction(new QAction(QIcon::fromTheme("general_search"), tr("Retrieve URLs"), this)),
    m_clipboardUrlsAction(new QAction(QIcon::fromTheme("general_share"), tr("Clipboard URLs"), this)),
    m_downloadRequestAction(new QAction(QIcon::fromTheme("notes_save"), tr("Retrieve download requests"), this)),
    m_queueAction(new QAction(QIcon("/etc/hildon/theme/mediaplayer/Play.png"), tr("Start all DLs"), this)),
    m_pauseAction(new QAction(QIcon("/etc/hildon/theme/mediaplayer/Pause.png"), tr("Pause all DLs"), this)),
    m_propertiesAction(new QAction(QIcon::fromTheme("general_information"), tr("Properties"), this)),
    m_transferQueueAction(new QAction(tr("Start"), this)),
    m_transferPauseAction(new QAction(tr("Pause"), this)),
    m_transferCancelAction(new QAction(tr("Remove"), this)),
    m_transferCancelDeleteAction(new QAction(tr("Remove and delete files"), this)),
    m_packageQueueAction(new QAction(tr("Start"), this)),
    m_packagePauseAction(new QAction(tr("Pause"), this)),
    m_packageCancelAction(new QAction(tr("Remove"), this)),
    m_packageCancelDeleteAction(new QAction(tr("Remove and delete files"), this)),
    m_widget(new QWidget(this)),
    m_tabs(new QTabBar(m_widget)),
    m_stack(new QStackedWidget(m_widget)),
    m_view(new QTreeView(m_stack)),
    m_toolBar(new QToolBar(this)),
    m_messageLabel(new QLabel(this)),
    m_layout(new QVBoxLayout(m_widget))
{
    setWindowTitle(QString("%1DLs | %2").arg(TransferModel::instance()->activeTransfers())
            .arg(TransferModel::instance()->totalSpeedString()));
    setCentralWidget(m_widget);
    addToolBar(Qt::BottomToolBarArea, m_toolBar);
    addAction(m_searchAction);
    addAction(m_pluginsAction);
    addAction(m_closePageAction);
    addAction(m_firstPageAction);
    addAction(m_lastPageAction);
    addAction(m_nextPageAction);
    addAction(m_previousPageAction);
    addAction(m_scrollBottomAction);
    addAction(m_scrollTopAction);

    menuBar()->addAction(m_concurrentAction);
    menuBar()->addAction(m_nextAction);
    menuBar()->addAction(m_searchAction);
    menuBar()->addAction(m_settingsAction);
    menuBar()->addAction(m_pluginsAction);
    menuBar()->addAction(m_aboutAction);
    
    m_concurrentAction->setModel(new ConcurrentTransfersModel(m_concurrentAction));
    m_concurrentAction->setValue(Settings::maximumConcurrentTransfers());
    m_nextAction->setModel(new ActionModel(m_nextAction));
    m_nextAction->setValue(Settings::nextAction());
    m_searchAction->setShortcut(tr("Ctrl+S"));
    m_searchAction->setEnabled(SearchPluginManager::instance()->count() > 0);
    m_pluginsAction->setShortcut(tr("Ctrl+L"));
    
    m_closePageAction->setShortcut(tr("Ctrl+W"));
    m_firstPageAction->setShortcut(tr("Ctrl+Up"));
    m_lastPageAction->setShortcut(tr("Ctrl+Down"));
    m_nextPageAction->setShortcut(tr("Ctrl+Right"));
    m_previousPageAction->setShortcut(tr("Ctrl+Left"));
    m_scrollBottomAction->setShortcut(tr("Shift+Down"));
    m_scrollTopAction->setShortcut(tr("Shift+Up"));
    
    m_addUrlsAction->setShortcut(tr("Ctrl+N"));
    m_importUrlsAction->setShortcut(tr("Ctrl+O"));
    m_retrieveUrlsAction->setShortcut(tr("Ctrl+F"));
    m_clipboardUrlsAction->setShortcut(tr("Ctrl+U"));
    m_downloadRequestAction->setShortcut(tr("Ctrl+D"));
    m_propertiesAction->setShortcut(tr("Ctrl+I"));
    m_propertiesAction->setEnabled(false);

    m_transferMenu->addAction(m_transferQueueAction);
    m_transferMenu->addAction(m_transferPauseAction);
    m_transferMenu->addAction(m_transferCancelAction);
    m_transferMenu->addAction(m_transferCancelDeleteAction);

    m_packageMenu->addAction(m_packageQueueAction);
    m_packageMenu->addAction(m_packagePauseAction);
    m_packageMenu->addAction(m_packageCancelAction);
    m_packageMenu->addAction(m_packageCancelDeleteAction);
    
    m_tabs->setTabsClosable(true);
    m_tabs->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    m_tabs->setExpanding(false);
    m_tabs->setStyleSheet("QTabBar::tab { height: 40px; }");
    m_tabs->addTab(tr("Downloads"));
    m_tabs->hide();
    
    if (QWidget *button = m_tabs->tabButton(0, QTabBar::RightSide)) {
        button->hide();
    }
    
    m_stack->addWidget(m_view);
    
    m_view->setModel(TransferModel::instance());
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(true);
    m_view->setItemsExpandable(true);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);

    QHeaderView *header = m_view->header();
    
    if (!header->restoreState(Settings::transferViewHeaderState())) {
        const QFontMetrics fm = header->fontMetrics();
        header->resizeSection(0, 200);
        header->resizeSection(3, fm.width("999.99MB of 999.99MB (99.99%)"));
//        header->resizeSection(4, fm.width("999.99KB/s"));
    }

    header->hideSection(1); // Hide category column
    header->hideSection(2); // Hide priority column
    header->hideSection(4); // Hide speed column

    m_messageLabel->setMargin(8);

    m_toolBar->setAllowedAreas(Qt::BottomToolBarArea);
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_toolBar->setMovable(false);
    m_toolBar->addAction(m_addUrlsAction);
    m_toolBar->addAction(m_importUrlsAction);
    m_toolBar->addAction(m_retrieveUrlsAction);
    m_toolBar->addAction(m_clipboardUrlsAction);
    m_toolBar->addAction(m_downloadRequestAction);
    m_toolBar->addAction(m_queueAction);
    m_toolBar->addAction(m_pauseAction);
    m_toolBar->addAction(m_propertiesAction);
    m_toolBar->addWidget(m_messageLabel);
    
    m_layout->addWidget(m_tabs);
    m_layout->addWidget(m_stack);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int)),
            this, SLOT(onMaximumConcurrentTransfersChanged(int)));
    connect(Settings::instance(), SIGNAL(nextActionChanged(int)), this, SLOT(onNextActionChanged(int)));
    
    connect(TransferModel::instance(), SIGNAL(captchaRequest(TransferItem*)), this, SLOT(showCaptchaDialog(TransferItem*)));
    connect(TransferModel::instance(), SIGNAL(settingsRequest(TransferItem*)), this, SLOT(showPluginSettingsDialog(TransferItem*)));
    connect(TransferModel::instance(), SIGNAL(activeTransfersChanged(int)), this, SLOT(onActiveTransfersChanged(int)));
    connect(TransferModel::instance(), SIGNAL(totalSpeedChanged(int)), this, SLOT(onTotalSpeedChanged(int)));

    connect(m_transferMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    connect(m_packageMenu, SIGNAL(aboutToShow()), this, SLOT(setPackageMenuActions()));
    
    connect(m_concurrentAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setMaximumConcurrentTransfers(QVariant)));
    connect(m_nextAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setNextAction(QVariant)));
    connect(m_searchAction, SIGNAL(triggered()), this, SLOT(showSearchDialog()));
    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    connect(m_pluginsAction, SIGNAL(triggered()), this, SLOT(loadPlugins()));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    
    connect(m_closePageAction, SIGNAL(triggered()), this, SLOT(closeCurrentPage()));
    connect(m_firstPageAction, SIGNAL(triggered()), this, SLOT(showFirstPage()));
    connect(m_lastPageAction, SIGNAL(triggered()), this, SLOT(showLastPage()));
    connect(m_nextPageAction, SIGNAL(triggered()), this, SLOT(showNextPage()));
    connect(m_previousPageAction, SIGNAL(triggered()), this, SLOT(showPreviousPage()));
    connect(m_scrollBottomAction, SIGNAL(triggered()), m_view, SLOT(scrollToBottom()));
    connect(m_scrollTopAction, SIGNAL(triggered()), m_view, SLOT(scrollToTop()));

    connect(m_addUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    connect(m_importUrlsAction, SIGNAL(triggered()), this, SLOT(showImportUrlsDialog()));
    connect(m_retrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    connect(m_clipboardUrlsAction, SIGNAL(triggered()), this, SLOT(showClipboardUrlsDialog()));
    connect(m_downloadRequestAction, SIGNAL(triggered()), this, SLOT(showDownloadRequestDialog()));
    connect(m_queueAction, SIGNAL(triggered()), TransferModel::instance(), SLOT(queue()));
    connect(m_pauseAction, SIGNAL(triggered()), TransferModel::instance(), SLOT(pause()));
    connect(m_propertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentItemProperties()));
    
    connect(m_transferQueueAction, SIGNAL(triggered()), this, SLOT(queueCurrentTransfer()));
    connect(m_transferPauseAction, SIGNAL(triggered()), this, SLOT(pauseCurrentTransfer()));
    connect(m_transferCancelAction, SIGNAL(triggered()), this, SLOT(cancelCurrentTransfer()));
    connect(m_transferCancelDeleteAction, SIGNAL(triggered()), this, SLOT(cancelAndDeleteCurrentTransfer()));

    connect(m_packageQueueAction, SIGNAL(triggered()), this, SLOT(queueCurrentPackage()));
    connect(m_packagePauseAction, SIGNAL(triggered()), this, SLOT(pauseCurrentPackage()));
    connect(m_packageCancelAction, SIGNAL(triggered()), this, SLOT(cancelCurrentPackage()));
    connect(m_packageCancelDeleteAction, SIGNAL(triggered()), this, SLOT(cancelAndDeleteCurrentPackage()));
    
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(setCurrentPage(int)));
    connect(m_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closePage(int)));

    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentRowChanged(QModelIndex)));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (TransferModel::instance()->activeTransfers() > 0) {
        if (QMessageBox::question(this, tr("Quit?"),
            tr("Some downloads are still active. Do you want to quit?"),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    
    Settings::setTransferViewHeaderState(m_view->header()->saveState());
    QMainWindow::closeEvent(event);
    Qdl::quit();
}

void MainWindow::queueCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Queued, TransferItem::StatusRole);
    }
}

void MainWindow::pauseCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Paused, TransferItem::StatusRole);
    }
}

void MainWindow::cancelCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        if (QMessageBox::question(this, tr("Remove?"), tr("Do you want to remove download '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Canceled,
                                               TransferItem::StatusRole);
        }
    }
}

void MainWindow::cancelAndDeleteCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        if (QMessageBox::question(this, tr("Delete files?"), tr("Do you want to delete the files for download '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::CanceledAndDeleted,
                                               TransferItem::StatusRole);
        }
    }
}

void MainWindow::queueCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Queued, TransferItem::StatusRole);
    }
}

void MainWindow::pauseCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Paused, TransferItem::StatusRole);
    }
}

void MainWindow::cancelCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (QMessageBox::question(this, tr("Remove?"), tr("Do you want to remove package '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Canceled,
                                               TransferItem::StatusRole);
        }
    }
}

void MainWindow::cancelAndDeleteCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (QMessageBox::question(this, tr("Delete files?"), tr("Do you want to delete the files for package '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::CanceledAndDeleted,
                                               TransferItem::StatusRole);
        }
    }
}

void MainWindow::showCurrentItemProperties() {
    switch (m_view->currentIndex().data(TransferItem::ItemTypeRole).toInt()) {
    case TransferItem::PackageType:
        if (TransferItem *package = TransferModel::instance()->get(m_view->currentIndex())) {
            PackagePropertiesDialog(package, this).exec();
        }

        break;
    case TransferItem::TransferType:
        if (TransferItem *transfer = TransferModel::instance()->get(m_view->currentIndex())) {
            TransferPropertiesDialog(transfer, this).exec();
        }

        break;
    default:
        break;
    }
}

void MainWindow::setMaximumConcurrentTransfers(const QVariant &maximum) {
    Settings::setMaximumConcurrentTransfers(maximum.toInt());
}

void MainWindow::setNextAction(const QVariant &action) {
    Settings::setNextAction(action.toInt());
}

void MainWindow::setTransferMenuActions() {
    const bool canCancel = m_view->currentIndex().data(TransferItem::CanCancelRole).toBool();
    m_transferQueueAction->setEnabled(m_view->currentIndex().data(TransferItem::CanStartRole).toBool());
    m_transferPauseAction->setEnabled(m_view->currentIndex().data(TransferItem::CanPauseRole).toBool());
    m_transferCancelAction->setEnabled(canCancel);
    m_transferCancelDeleteAction->setEnabled(canCancel);
}

void MainWindow::setPackageMenuActions() {
    const bool canCancel = m_view->currentIndex().data(TransferItem::CanCancelRole).toBool();
    m_packageQueueAction->setEnabled(m_view->currentIndex().data(TransferItem::CanStartRole).toBool());
    m_packagePauseAction->setEnabled(m_view->currentIndex().data(TransferItem::CanPauseRole).toBool());
    m_packageCancelAction->setEnabled(canCancel);
    m_packageCancelDeleteAction->setEnabled(canCancel);
}

void MainWindow::showContextMenu(const QPoint &pos) {
    if (!m_view->currentIndex().isValid()) {
        return;
    }

    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        m_packageMenu->popup(m_view->mapToGlobal(pos), m_packageQueueAction);
    }
    else {
        m_transferMenu->popup(m_view->mapToGlobal(pos), m_transferQueueAction);
    }
}

void MainWindow::closePage(int index) {
    if (index > 0) {
        if (QWidget *page = m_stack->widget(index)) {
            m_stack->removeWidget(page);
            m_tabs->removeTab(index);
            page->close();
            
            if (m_tabs->count() == 1) {
                m_tabs->hide();
            }
        }
    }
}

void MainWindow::closeCurrentPage() {
    closePage(m_tabs->currentIndex());
}

void MainWindow::setCurrentPage(int index) {
    m_stack->setCurrentIndex(index);
    m_closePageAction->setEnabled(index > 0);

    if (const Page *page = qobject_cast<Page*>(m_stack->currentWidget())) {
        showMessage(page->statusString());
    }
    else {
        showMessage(QString("%1DLs").arg(TransferModel::instance()->activeTransfers()));
    }
}

void MainWindow::showFirstPage() {
    m_tabs->setCurrentIndex(0);
}

void MainWindow::showLastPage() {
    m_tabs->setCurrentIndex(m_tabs->count() - 1);
}

void MainWindow::showNextPage() {
    m_tabs->setCurrentIndex(m_tabs->currentIndex() + 1);
}

void MainWindow::showPreviousPage() {
    m_tabs->setCurrentIndex(m_tabs->currentIndex() - 1);
}

void MainWindow::search(const QString &pluginName, const QString &pluginId) {
    SearchPage *page = new SearchPage(m_stack);
    page->setAttribute(Qt::WA_DeleteOnClose, true);
    m_stack->addWidget(page);
    const int index = m_stack->indexOf(page);
    m_tabs->addTab(pluginName);
    m_stack->setCurrentIndex(index);
    m_tabs->setCurrentIndex(index);
    m_tabs->show();
    connect(page, SIGNAL(statusChanged(Page::Status)), this, SLOT(onPageStatusChanged()));
    page->search(pluginId);
}

void MainWindow::showSearchDialog() {
    SearchDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        search(dialog.pluginName(), dialog.pluginId());
    }
}

void MainWindow::showSettingsDialog() {
    SettingsDialog(this).exec();
}

void MainWindow::showAboutDialog() {
    AboutDialog(this).exec();
}

void MainWindow::showAddUrlsDialog() {
    AddUrlsDialog addDialog(this);

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

void MainWindow::showAddUrlsDialog(const QStringList &urls) {
    AddUrlsDialog addDialog(this);
    addDialog.setUrls(urls);

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

void MainWindow::showImportUrlsDialog() {
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import URLs"), HOME_PATH, "*.txt");
    
    if (!filePath.isEmpty()) {
        AddUrlsDialog addDialog(this);
        addDialog.importUrls(filePath);

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
                            addDialog.postData(), addDialog.category(), addDialog.createSubfolder(),
                            addDialog.priority(), addDialog.customCommand(), addDialog.customCommandOverrideEnabled());
                }
            }
        }
    }
}

void MainWindow::showRetrieveUrlsDialog() {
    RetrieveUrlsDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList results = dialog.results();
        dialog.clear();

        if (!results.isEmpty()) {
            TextEditDialog dialog(results.join("\n"), this);
            dialog.setWindowTitle(tr("Retrieve URLs"));
            dialog.setLabelText(tr("Results"));
            dialog.exec();
        }
    }
}

void MainWindow::showRetrieveUrlsDialog(const QStringList &urls) {
    RetrieveUrlsDialog dialog(this);
    dialog.setUrls(urls);

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList results = dialog.results();
        dialog.clear();
        
        if (!results.isEmpty()) {
            TextEditDialog dialog(results.join("\n"), this);
            dialog.setWindowTitle(tr("Retrieve URLs"));
            dialog.setLabelText(tr("Results"));
            dialog.exec();
        }
    }
}

void MainWindow::showClipboardUrlsDialog() {
    ClipboardUrlsDialog(this).exec();
}

void MainWindow::showDownloadRequestDialog() {
    const QStringList urls = TextEditDialog::getText(this, tr("Retrieve download requests"), tr("URLs"))
        .split("\n", QString::SkipEmptyParts);

    if (!urls.isEmpty()) {
        showDownloadRequestDialog(urls);
    }
}

void MainWindow::showDownloadRequestDialog(const QStringList &urls) {
    DownloadRequestDialog dialog(this);
    dialog.addUrls(urls);
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

void MainWindow::showCaptchaDialog(TransferItem *t) {
    QPointer<TransferItem> transfer(t);
    const int captchaType = transfer->data(TransferItem::CaptchaTypeRole).toInt();

    if (captchaType == CaptchaType::NoCaptcha) {
        NoCaptchaDialog dialog(this);
        dialog.setHtml(transfer->data(TransferItem::CaptchaDataRole).toString(),
                transfer->data(TransferItem::UrlRole).toString());
        dialog.setTimeout(transfer->data(TransferItem::CaptchaTimeoutRole).toInt());
        connect(transfer, SIGNAL(finished(TransferItem*)), &dialog, SLOT(close()));
        
        switch (dialog.exec()) {
        case QDialog::Accepted:
            if (transfer) {
                transfer->setData(TransferItem::CaptchaResponseRole, dialog.response());
            }
            
            break;
        case QDialog::Rejected:
            if (transfer) {
                transfer->setData(TransferItem::CaptchaResponseRole, QString());
            }
            
            break;
        default:
            break;
        }
    }
    else {
        CaptchaDialog dialog(this);
        dialog.setImage(QImage::fromData(QByteArray::fromBase64(transfer->data(TransferItem::CaptchaDataRole)
                        .toByteArray())));
        dialog.setTimeout(transfer->data(TransferItem::CaptchaTimeoutRole).toInt());
        connect(transfer, SIGNAL(finished(TransferItem*)), &dialog, SLOT(close()));
        
        switch (dialog.exec()) {
        case QDialog::Accepted:
            if (transfer) {
                transfer->setData(TransferItem::CaptchaResponseRole, dialog.response());
            }
            
            break;
        case QDialog::Rejected:
            if (transfer) {
                transfer->setData(TransferItem::CaptchaResponseRole, QString());
            }
            
            break;
        default:
            break;
        }
    }
}

void MainWindow::showPluginSettingsDialog(TransferItem *t) {
    QPointer<TransferItem> transfer(t);
    PluginSettingsDialog dialog(transfer->data(TransferItem::RequestedSettingsRole).toList(), this);
    dialog.setWindowTitle(transfer->data(TransferItem::RequestedSettingsTitleRole).toString());
    dialog.setTimeout(transfer->data(TransferItem::RequestedSettingsTimeoutRole).toInt());
    connect(transfer, SIGNAL(finished(TransferItem*)), &dialog, SLOT(close()));
    
    switch (dialog.exec()) {
    case QDialog::Accepted:
        if (transfer) {
            transfer->setData(TransferItem::RequestedSettingsRole, dialog.settings());
        }
        
        break;
    case QDialog::Rejected:
        if (transfer) {
            transfer->setData(TransferItem::RequestedSettingsRole, QVariant());
        }
        
        break;
    default:
        break;
    }
}

void MainWindow::showMessage(const QString &message) {
    m_messageLabel->setText(message);
}

void MainWindow::showError(const QString &errorString) {
    QMessageBox::critical(this, tr("Error"), errorString);
}

void MainWindow::loadPlugins() {
    const int decaptcha = DecaptchaPluginManager::instance()->load();
    const int recaptcha = RecaptchaPluginManager::instance()->load();
    const int search = SearchPluginManager::instance()->load();
    const int services = ServicePluginManager::instance()->load();
    const int count = decaptcha + recaptcha + search + services;

    if (count > 0) {
        showMessage(tr("%1 new plugin(s)").arg(count));
        
        if (search > 0) {
            m_searchAction->setEnabled(true);
        }
    }
    else {
        showMessage(tr("No new plugins"));
    }
}

void MainWindow::onActiveTransfersChanged(int active) {
    setWindowTitle(QString("%1DLs | %2").arg(active).arg(TransferModel::instance()->totalSpeedString()));
}

void MainWindow::onCurrentRowChanged(const QModelIndex &index) {
    m_propertiesAction->setEnabled(index.isValid());
}

void MainWindow::onMaximumConcurrentTransfersChanged(int maximum) {
    m_concurrentAction->setValue(maximum);
}

void MainWindow::onNextActionChanged(int action) {
    m_nextAction->setValue(action);
}

void MainWindow::onPageStatusChanged() {
    if (m_stack->currentIndex() > 0) {
        if (const Page *page = qobject_cast<Page*>(sender())) {
            switch (page->status()) {
            case Page::Error:
                showError(page->errorString());
                break;
            default:
                break;
            }

            showMessage(page->statusString());
        }
    }
}

void MainWindow::onTotalSpeedChanged(int speed) {
    setWindowTitle(QString("%1DLs | %2/s").arg(TransferModel::instance()->activeTransfers())
            .arg(Utils::formatBytes(speed)));
}
