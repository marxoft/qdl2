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

#include "mainwindow.h"
#include "aboutdialog.h"
#include "actionmodel.h"
#include "addurlsdialog.h"
#include "captchadialog.h"
#include "captchatype.h"
#include "categories.h"
#include "clipboardurlsdialog.h"
#include "concurrenttransfersmodel.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "downloadrequestdialog.h"
#include "nocaptchadialog.h"
#include "packagepropertiesdialog.h"
#include "pluginsettings.h"
#include "pluginsettingsdialog.h"
#include "qdl.h"
#include "recaptchapluginmanager.h"
#include "retrieveurlsdialog.h"
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
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_transferMenu(new QMenu(tr("Download"), this)),
    m_packageMenu(new QMenu(tr("Package"), this)),
    m_concurrentAction(new ValueSelectorAction(tr("Maximum concurrent DLs"), this)),
    m_nextAction(new ValueSelectorAction(tr("After current DLs"), this)),
    m_settingsAction(new QAction(tr("Settings"), this)),
    m_reloadAction(new QAction(tr("Reload data"), this)),
    m_aboutAction(new QAction(tr("About"), this)),
    m_topAction(new QAction(tr("Scroll to top"), this)),
    m_bottomAction(new QAction(tr("Scroll to bottom"), this)),
    m_addUrlsAction(new QAction(QIcon::fromTheme("general_add"), tr("Add URLs"), this)),
    m_importUrlsAction(new QAction(QIcon::fromTheme("general_toolbar_folder"), tr("Import URLs"), this)),
    m_retrieveUrlsAction(new QAction(QIcon::fromTheme("general_search"), tr("Retrieve URLs"), this)),
    m_clipboardUrlsAction(new QAction(QIcon::fromTheme("general_share"), tr("Clipboard URLs"), this)),
    m_downloadRequestAction(new QAction(QIcon::fromTheme("notes_save"), tr("Retrieve download requests"), this)),
    m_queueAction(new QAction(QIcon("/etc/hildon/theme/mediaplayer/Play.png"), tr("Start all DLs"), this)),
    m_pauseAction(new QAction(QIcon("/etc/hildon/theme/mediaplayer/Pause.png"), tr("Pause all DLs"), this)),
    m_autoReloadAction(new QAction(QIcon::fromTheme("general_refresh"), tr("Reload active downloads automatically"), this)),
    m_propertiesAction(new QAction(QIcon::fromTheme("general_information"), tr("Properties"), this)),
    m_transferReloadAction(new QAction(tr("Reload"), this)),
    m_transferQueueAction(new QAction(tr("Start"), this)),
    m_transferPauseAction(new QAction(tr("Pause"), this)),
    m_transferCancelAction(new QAction(tr("Remove"), this)),
    m_transferCancelDeleteAction(new QAction(tr("Remove and delete files"), this)),
    m_packageReloadAction(new QAction(tr("Reload"), this)),
    m_packageQueueAction(new QAction(tr("Start"), this)),
    m_packagePauseAction(new QAction(tr("Pause"), this)),
    m_packageCancelAction(new QAction(tr("Remove"), this)),
    m_packageCancelDeleteAction(new QAction(tr("Remove and delete files"), this)),
    m_view(new QTreeView(this)),
    m_toolBar(new QToolBar(this))
{
    setWindowTitle(QString("%1DLs | %2").arg(TransferModel::instance()->activeTransfers())
            .arg(TransferModel::instance()->totalSpeedString()));
    setCentralWidget(m_view);
    addToolBar(Qt::BottomToolBarArea, m_toolBar);
    addAction(m_reloadAction);
    addAction(m_topAction);
    addAction(m_bottomAction);

    menuBar()->addAction(m_concurrentAction);
    menuBar()->addAction(m_nextAction);
    menuBar()->addAction(m_settingsAction);
    menuBar()->addAction(m_reloadAction);
    menuBar()->addAction(m_aboutAction);
    
    m_concurrentAction->setModel(new ConcurrentTransfersModel(m_concurrentAction));
    m_concurrentAction->setValue(Settings::instance()->maximumConcurrentTransfers());
    m_nextAction->setModel(new ActionModel(m_nextAction));
    m_nextAction->setValue(Settings::instance()->nextAction());
    m_reloadAction->setShortcut(tr("Ctrl+L"));
    
    m_topAction->setShortcut(tr("Shift+Up"));
    m_bottomAction->setShortcut(tr("Shift+Down"));
    
    m_addUrlsAction->setShortcut(tr("Ctrl+N"));
    m_importUrlsAction->setShortcut(tr("Ctrl+O"));
    m_retrieveUrlsAction->setShortcut(tr("Ctrl+F"));
    m_clipboardUrlsAction->setShortcut(tr("Ctrl+U"));
    m_downloadRequestAction->setShortcut(tr("Ctrl+D"));
    m_autoReloadAction->setCheckable(true);
    m_autoReloadAction->setChecked(Settings::autoReloadEnabled());
    m_autoReloadAction->setShortcut(tr("Ctrl+R"));
    m_propertiesAction->setShortcut(tr("Ctrl+I"));
    m_propertiesAction->setEnabled(false);

    m_transferMenu->addAction(m_transferReloadAction);
    m_transferMenu->addAction(m_transferQueueAction);
    m_transferMenu->addAction(m_transferPauseAction);
    m_transferMenu->addAction(m_transferCancelAction);
    m_transferMenu->addAction(m_transferCancelDeleteAction);

    m_packageMenu->addAction(m_packageReloadAction);
    m_packageMenu->addAction(m_packageQueueAction);
    m_packageMenu->addAction(m_packagePauseAction);
    m_packageMenu->addAction(m_packageCancelAction);
    m_packageMenu->addAction(m_packageCancelDeleteAction);
    
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
    m_toolBar->addAction(m_autoReloadAction);
    m_toolBar->addAction(m_propertiesAction);
    
    connect(Categories::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));

    connect(PluginSettings::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));

    connect(Settings::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int)),
            this, SLOT(onMaximumConcurrentTransfersChanged(int)));
    connect(Settings::instance(), SIGNAL(nextActionChanged(int)), this, SLOT(onNextActionChanged(int)));
    
    connect(TransferModel::instance(), SIGNAL(captchaRequest(TransferItem*)), this, SLOT(showCaptchaDialog(TransferItem*)));
    connect(TransferModel::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(TransferModel::instance(), SIGNAL(settingsRequest(TransferItem*)), this, SLOT(showPluginSettingsDialog(TransferItem*)));
    connect(TransferModel::instance(), SIGNAL(activeTransfersChanged(int)), this, SLOT(onActiveTransfersChanged(int)));
    connect(TransferModel::instance(), SIGNAL(totalSpeedChanged(int)), this, SLOT(onTotalSpeedChanged(int)));

    connect(m_transferMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    connect(m_packageMenu, SIGNAL(aboutToShow()), this, SLOT(setPackageMenuActions()));
    
    connect(m_concurrentAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setMaximumConcurrentTransfers(QVariant)));
    connect(m_nextAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setNextAction(QVariant)));
    connect(m_queueAction, SIGNAL(triggered()), TransferModel::instance(), SLOT(queue()));
    connect(m_pauseAction, SIGNAL(triggered()), TransferModel::instance(), SLOT(pause()));
    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    connect(m_reloadAction, SIGNAL(triggered()), this, SLOT(reload()));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    
    connect(m_topAction, SIGNAL(triggered()), m_view, SLOT(scrollToTop()));
    connect(m_bottomAction, SIGNAL(triggered()), m_view, SLOT(scrollToBottom()));

    connect(m_addUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    connect(m_importUrlsAction, SIGNAL(triggered()), this, SLOT(showImportUrlsDialog()));
    connect(m_retrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    connect(m_clipboardUrlsAction, SIGNAL(triggered()), this, SLOT(showClipboardUrlsDialog()));
    connect(m_downloadRequestAction, SIGNAL(triggered()), this, SLOT(showDownloadRequestDialog()));
    connect(m_autoReloadAction, SIGNAL(toggled(bool)), Settings::instance(), SLOT(setAutoReloadEnabled(bool)));
    connect(m_propertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentItemProperties()));
    
    connect(m_transferReloadAction, SIGNAL(triggered()), this, SLOT(reloadCurrentTransfer()));
    connect(m_transferQueueAction, SIGNAL(triggered()), this, SLOT(queueCurrentTransfer()));
    connect(m_transferPauseAction, SIGNAL(triggered()), this, SLOT(pauseCurrentTransfer()));
    connect(m_transferCancelAction, SIGNAL(triggered()), this, SLOT(cancelCurrentTransfer()));
    connect(m_transferCancelDeleteAction, SIGNAL(triggered()), this, SLOT(cancelAndDeleteCurrentTransfer()));

    connect(m_packageReloadAction, SIGNAL(triggered()), this, SLOT(reloadCurrentPackage()));
    connect(m_packageQueueAction, SIGNAL(triggered()), this, SLOT(queueCurrentPackage()));
    connect(m_packagePauseAction, SIGNAL(triggered()), this, SLOT(pauseCurrentPackage()));
    connect(m_packageCancelAction, SIGNAL(triggered()), this, SLOT(cancelCurrentPackage()));
    connect(m_packageCancelDeleteAction, SIGNAL(triggered()), this, SLOT(cancelAndDeleteCurrentPackage()));
    
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentRowChanged(QModelIndex)));

    if (!Settings::serverHost().isEmpty()) {
        reload();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Settings::setTransferViewHeaderState(m_view->header()->saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::reloadCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        TransferModel::instance()->reload(m_view->currentIndex());
    }
}

void MainWindow::queueCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        TransferModel::instance()->queue(m_view->currentIndex());
    }
}

void MainWindow::pauseCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        TransferModel::instance()->pause(m_view->currentIndex());
    }
}

void MainWindow::cancelCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        if (QMessageBox::question(this, tr("Remove?"), tr("Do you want to remove download '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->cancel(m_view->currentIndex());
        }
    }
}

void MainWindow::cancelAndDeleteCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        if (QMessageBox::question(this, tr("Delete files?"), tr("Do you want to delete the files for download '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->cancel(m_view->currentIndex(), true);
        }
    }
}

void MainWindow::reloadCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->reload(m_view->currentIndex());
    }
}

void MainWindow::queueCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->queue(m_view->currentIndex());
    }
}

void MainWindow::pauseCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->pause(m_view->currentIndex());
    }
}

void MainWindow::cancelCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (QMessageBox::question(this, tr("Remove?"), tr("Do you want to remove package '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->cancel(m_view->currentIndex());
        }
    }
}

void MainWindow::cancelAndDeleteCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (QMessageBox::question(this, tr("Delete files?"), tr("Do you want to delete the files for package '%1'?")
                                  .arg(m_view->currentIndex().data(TransferItem::NameRole).toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            TransferModel::instance()->cancel(m_view->currentIndex(), true);
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
    Settings::instance()->setMaximumConcurrentTransfers(maximum.toInt());
}

void MainWindow::setNextAction(const QVariant &action) {
    Settings::instance()->setNextAction(action.toInt());
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
        m_packageMenu->popup(m_view->mapToGlobal(pos), m_packageReloadAction);
    }
    else {
        m_transferMenu->popup(m_view->mapToGlobal(pos), m_transferReloadAction);
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
        connect(transfer, SIGNAL(statusChanged(TransferItem*, TransferItem::Status)), &dialog, SLOT(close()));
        
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
        connect(transfer, SIGNAL(statusChanged(TransferItem*, TransferItem::Status)), &dialog, SLOT(close()));
        
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
    connect(transfer, SIGNAL(statusChanged(TransferItem*, TransferItem::Status)), &dialog, SLOT(close()));
    
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

void MainWindow::showError(const QString &errorString) {
    QMessageBox::critical(this, tr("Error"), errorString);
}

void MainWindow::reload() {
    if (Settings::serverHost().isEmpty()) {
        showError(tr("No server host specified"));
        return;
    }

    Categories::instance()->load();
    Settings::instance()->restore();
    DecaptchaPluginManager::instance()->load();
    RecaptchaPluginManager::instance()->load();
    ServicePluginManager::instance()->load();
    TransferModel::instance()->reload();
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

void MainWindow::onTotalSpeedChanged(int speed) {
    setWindowTitle(QString("%1DLs | %2/s").arg(TransferModel::instance()->activeTransfers())
            .arg(Utils::formatBytes(speed)));
}
