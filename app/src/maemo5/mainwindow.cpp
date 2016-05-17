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
#include "clipboardurlsdialog.h"
#include "concurrenttransfersmodel.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "packagepropertiesdialog.h"
#include "qdl.h"
#include "recaptchapluginmanager.h"
#include "retrieveurlsdialog.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "settingsdialog.h"
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
#include <QMaemo5InformationBox>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTreeView>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_transferMenu(new QMenu(tr("Download"), this)),
    m_packageMenu(new QMenu(tr("Package"), this)),
    m_addUrlsAction(new QAction(QIcon::fromTheme("general_add"), tr("Add URLs"), this)),
    m_importUrlsAction(new QAction(QIcon::fromTheme("general_toolbar_folder"), tr("Import URLs"), this)),
    m_retrieveUrlsAction(new QAction(QIcon::fromTheme("general_search"), tr("Retrieve URLs"), this)),
    m_clipboardUrlsAction(new QAction(QIcon::fromTheme("general_share"), tr("Clipboard URLs"), this)),
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
    m_settingsAction(new QAction(tr("Settings"), this)),
    m_pluginsAction(new QAction(tr("Load plugins"), this)),
    m_aboutAction(new QAction(tr("About"), this)),
    m_concurrentAction(new ValueSelectorAction(tr("Maximum concurrent DLs"), this)),
    m_nextAction(new ValueSelectorAction(tr("After current DLs"), this)),
    m_view(new QTreeView(this)),
    m_toolBar(new QToolBar(this)),
    m_activeLabel(new QLabel(QString("%1DLs").arg(TransferModel::instance()->activeTransfers()), this)),
    m_speedLabel(new QLabel(Utils::formatBytes(TransferModel::instance()->totalSpeed()) + "/s", this))
{
    setWindowTitle("QDL");
    setCentralWidget(m_view);
    addToolBar(Qt::BottomToolBarArea, m_toolBar);

    menuBar()->addAction(m_concurrentAction);
    menuBar()->addAction(m_nextAction);
    menuBar()->addAction(m_queueAction);
    menuBar()->addAction(m_pauseAction);
    menuBar()->addAction(m_settingsAction);
    menuBar()->addAction(m_pluginsAction);
    menuBar()->addAction(m_aboutAction);

    m_addUrlsAction->setShortcut(tr("Ctrl+A"));
    m_importUrlsAction->setShortcut(tr("Ctrl+O"));
    m_retrieveUrlsAction->setShortcut(tr("Ctrl+R"));
    m_propertiesAction->setShortcut(tr("Ctrl+I"));
    m_propertiesAction->setEnabled(false);
    m_clipboardUrlsAction->setShortcut(tr("Ctrl+U"));
    m_pluginsAction->setShortcut(tr("Ctrl+L"));

    m_transferMenu->addAction(m_transferQueueAction);
    m_transferMenu->addAction(m_transferPauseAction);
    m_transferMenu->addAction(m_transferCancelAction);
    m_transferMenu->addAction(m_transferCancelDeleteAction);

    m_packageMenu->addAction(m_packageQueueAction);
    m_packageMenu->addAction(m_packagePauseAction);
    m_packageMenu->addAction(m_packageCancelAction);
    m_packageMenu->addAction(m_packageCancelDeleteAction);

    m_concurrentAction->setModel(new ConcurrentTransfersModel(m_concurrentAction));
    m_concurrentAction->setValue(Settings::maximumConcurrentTransfers());
    m_nextAction->setModel(new ActionModel(m_nextAction));
    m_nextAction->setValue(Settings::nextAction());

    QLabel *speedIcon = new QLabel(m_toolBar);
    speedIcon->setPixmap(QIcon::fromTheme("general_received").pixmap(m_toolBar->iconSize()));

    QWidget *spacer1 = new QWidget(m_toolBar);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    QWidget *spacer2 = new QWidget(m_toolBar);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    m_activeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_speedLabel->setMinimumWidth(m_speedLabel->fontMetrics().width("9999.99MB/s"));
    m_speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_toolBar->setAllowedAreas(Qt::BottomToolBarArea);
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_toolBar->setMovable(false);
    m_toolBar->addAction(m_addUrlsAction);
    m_toolBar->addAction(m_importUrlsAction);
    m_toolBar->addAction(m_retrieveUrlsAction);
    m_toolBar->addAction(m_clipboardUrlsAction);
    m_toolBar->addAction(m_propertiesAction);
    m_toolBar->addWidget(spacer1);
    m_toolBar->addWidget(m_activeLabel);
    m_toolBar->addWidget(spacer2);
    m_toolBar->addWidget(m_speedLabel);
    m_toolBar->addWidget(speedIcon);

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
        header->resizeSection(2, fm.width("999.99MB of 999.99MB (99.99%)"));
        header->resizeSection(3, fm.width("999.99KB/s"));
        header->hideSection(1); // Hide priority column
    }
    
    connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int)),
            this, SLOT(onMaximumConcurrentTransfersChanged(int)));
    connect(Settings::instance(), SIGNAL(nextActionChanged(int)), this, SLOT(onNextActionChanged(int)));
    
    connect(TransferModel::instance(), SIGNAL(activeTransfersChanged(int)), this, SLOT(onActiveTransfersChanged(int)));
    connect(TransferModel::instance(), SIGNAL(totalSpeedChanged(int)), this, SLOT(onTotalSpeedChanged(int)));

    connect(m_transferMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    connect(m_packageMenu, SIGNAL(aboutToShow()), this, SLOT(setPackageMenuActions()));

    connect(m_addUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    connect(m_importUrlsAction, SIGNAL(triggered()), this, SLOT(showImportUrlsDialog()));
    connect(m_retrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    connect(m_clipboardUrlsAction, SIGNAL(triggered()), this, SLOT(showClipboardUrlsDialog()));
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

    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    connect(m_pluginsAction, SIGNAL(triggered()), this, SLOT(loadPlugins()));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    connect(m_concurrentAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setMaximumConcurrentTransfers(QVariant)));
    connect(m_nextAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setNextAction(QVariant)));

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

void MainWindow::showAddUrlsDialog() {
    AddUrlsDialog addDialog(this);

    if (addDialog.exec() == QDialog::Accepted) {
        const QStringList urls = addDialog.urls();

        if (!urls.isEmpty()) {
            if (Settings::checkUrls()) {
                UrlCheckDialog checkDialog(this);
                checkDialog.addUrls(urls, Settings::defaultServicePlugin());
                checkDialog.exec();
            }
            else {
                foreach (const QString &url, urls) {
                    TransferModel::instance()->append(url);
                }
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
            if (Settings::checkUrls()) {
                UrlCheckDialog checkDialog(this);
                checkDialog.addUrls(urls, Settings::defaultServicePlugin());
                checkDialog.exec();
            }
            else {
                foreach (const QString &url, urls) {
                    TransferModel::instance()->append(url);
                }
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
                if (Settings::checkUrls()) {
                    UrlCheckDialog checkDialog(this);
                    checkDialog.addUrls(urls, Settings::defaultServicePlugin());
                    checkDialog.exec();
                }
                else {
                    foreach (const QString &url, urls) {
                        TransferModel::instance()->append(url);
                    }
                }
            }
        }
    }
}

void MainWindow::showRetrieveUrlsDialog() {
    RetrieveUrlsDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList results = dialog.results();

        if (!results.isEmpty()) {
            showAddUrlsDialog(results);
        }
    }
}

void MainWindow::showRetrieveUrlsDialog(const QStringList &urls) {
    RetrieveUrlsDialog dialog(this);
    dialog.setUrls(urls);

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList results = dialog.results();

        if (!results.isEmpty()) {
            showAddUrlsDialog(results);
        }
    }
}

void MainWindow::showClipboardUrlsDialog() {
    ClipboardUrlsDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList urls = dialog.urls();

        if (!urls.isEmpty()) {
            if (dialog.action() == Qdl::RetrieveUrls) {
                showRetrieveUrlsDialog(urls);
            }
            else {
                showAddUrlsDialog(urls);
            }
        }
    }
}

void MainWindow::showSettingsDialog() {
    SettingsDialog(this).exec();
}

void MainWindow::showAboutDialog() {
    AboutDialog(this).exec();
}

void MainWindow::loadPlugins() {
    const int count = DecaptchaPluginManager::instance()->load() + RecaptchaPluginManager::instance()->load()
                      + ServicePluginManager::instance()->load();

    if (count > 0) {
        QMaemo5InformationBox::information(this, tr("%1 new plugins found").arg(count));
    }
    else {
        QMaemo5InformationBox::information(this, tr("No new plugins found"));
    }
}

void MainWindow::onActiveTransfersChanged(int active) {
    m_activeLabel->setText(QString("%1DLs").arg(active));
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
    m_speedLabel->setText(Utils::formatBytes(speed) + "/s");
}
