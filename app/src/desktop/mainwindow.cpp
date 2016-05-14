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
#include "categories.h"
#include "clipboardurlsdialog.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "packagepropertiesdialog.h"
#include "qdl.h"
#include "recaptchapluginmanager.h"
#include "retrieveurlsdialog.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "settingsdialog.h"
#include "transferdelegate.h"
#include "transfermodel.h"
#include "transferpropertiesdialog.h"
#include "urlcheckdialog.h"
#include "utils.h"
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTreeView>
#include <QToolBar>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_fileMenu(new QMenu(tr("&File"), this)),
    m_transferMenu(new QMenu(tr("&Download"), this)),
    m_packageMenu(new QMenu(tr("&Package"), this)),
    m_editMenu(new QMenu(tr("&Edit"), this)),
    m_helpMenu(new QMenu(tr("&Help"), this)),
    m_transferPriorityMenu(new QMenu(tr("&Priority"), this)),
    m_packageCategoryMenu(new QMenu(tr("&Category"), this)),
    m_packagePriorityMenu(new QMenu(tr("&Priority"), this)),
    m_optionsMenu(new QMenu(tr("&Options"), this)),
    m_concurrentTransfersMenu(new QMenu(tr("&Maximum concurrent downloads"), this)),
    m_topToolBar(new QToolBar(this)),
    m_bottomToolBar(new QToolBar(this)),
    m_addUrlsAction(new QAction(QIcon::fromTheme("list-add"), tr("&Add URLs"), this)),
    m_importUrlsAction(new QAction(QIcon::fromTheme("document-open"), tr("&Import URLs"), this)),
    m_retrieveUrlsAction(new QAction(QIcon::fromTheme("search"), tr("&Retrieve URLs"), this)),
    m_clipboardUrlsAction(new QAction(QIcon::fromTheme("edit-copy"), tr("Show &clipboard URLs"), this)),
    m_queueAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("&Start all downloads"), this)),
    m_pauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause all downloads"), this)),
    m_pluginsAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Load plugins"), this)),
    m_quitAction(new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this)),
    m_transferPropertiesAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Properties"), this)),
    m_transferCustomCommandAction(new QAction(QIcon::fromTheme("system-run"), tr("Set &custom command"), this)),
    m_transferQueueAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("&Start"), this)),
    m_transferPauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause"), this)),
    m_transferCancelAction(new QAction(QIcon::fromTheme("edit-delete"), tr("&Remove"), this)),
    m_transferCancelDeleteAction(new QAction(QIcon::fromTheme("edit-delete"), tr("Remove and &delete files"), this)),
    m_packagePropertiesAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Properties"), this)),
    m_packageSubfolderAction(new QAction(tr("Create sub&folder"), this)),
    m_packageQueueAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("&Start"), this)),
    m_packagePauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause"), this)),
    m_packageCancelAction(new QAction(QIcon::fromTheme("edit-delete"), tr("&Remove"), this)),
    m_packageCancelDeleteAction(new QAction(QIcon::fromTheme("edit-delete"), tr("Remove and &delete files"), this)),
    m_settingsAction(new QAction(QIcon::fromTheme("preferences-desktop"), tr("&Preferences"), this)),
    m_aboutAction(new QAction(QIcon::fromTheme("help-about"), tr("&About"), this)),
    m_transferPriorityGroup(new QActionGroup(this)),
    m_packageCategoryGroup(new QActionGroup(this)),
    m_packagePriorityGroup(new QActionGroup(this)),
    m_concurrentTransfersGroup(new QActionGroup(this)),
    m_actionSelector(new QComboBox(this)),
    m_view(new QTreeView(this)),
    m_optionsButton(new QToolButton(this)),
    m_activeLabel(new QLabel(QString("%1DLs").arg(TransferModel::instance()->activeTransfers()), this)),
    m_speedLabel(new QLabel(Utils::formatBytes(TransferModel::instance()->totalSpeed()) + "/s", this))
{
    setWindowTitle("QDL");
    setCentralWidget(m_view);
    setAcceptDrops(true);
    addToolBar(Qt::TopToolBarArea, m_topToolBar);
    addToolBar(Qt::BottomToolBarArea, m_bottomToolBar);

    if (!restoreGeometry(Settings::windowGeometry())) {
        resize(1000, 600);
    }

    restoreState(Settings::windowState());

    menuBar()->addMenu(m_fileMenu);
    menuBar()->addMenu(m_transferMenu);
    menuBar()->addMenu(m_packageMenu);
    menuBar()->addMenu(m_editMenu);
    menuBar()->addMenu(m_helpMenu);

    m_addUrlsAction->setShortcut(tr("Ctrl+A"));
    m_importUrlsAction->setShortcut(tr("Ctrl+O"));
    m_retrieveUrlsAction->setShortcut(tr("Ctrl+R"));
    m_clipboardUrlsAction->setShortcut(tr("Ctrl+U"));
    m_pluginsAction->setShortcut(tr("Ctrl+L"));
    m_quitAction->setShortcut(tr("Ctrl+Q"));

    m_fileMenu->addAction(m_addUrlsAction);
    m_fileMenu->addAction(m_importUrlsAction);
    m_fileMenu->addAction(m_retrieveUrlsAction);
    m_fileMenu->addAction(m_clipboardUrlsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_queueAction);
    m_fileMenu->addAction(m_pauseAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_pluginsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_transferMenu->addAction(m_transferPropertiesAction);
    m_transferMenu->addAction(m_transferCustomCommandAction);
    m_transferMenu->addAction(m_transferQueueAction);
    m_transferMenu->addAction(m_transferPauseAction);
    m_transferMenu->addMenu(m_transferPriorityMenu);
    m_transferMenu->addAction(m_transferCancelAction);
    m_transferMenu->addAction(m_transferCancelDeleteAction);
    m_transferMenu->setEnabled(false);

    m_packageMenu->addAction(m_packagePropertiesAction);
    m_packageMenu->addAction(m_packageSubfolderAction);
    m_packageMenu->addAction(m_packageQueueAction);
    m_packageMenu->addAction(m_packagePauseAction);
    m_packageMenu->addMenu(m_packageCategoryMenu);
    m_packageMenu->addMenu(m_packagePriorityMenu);
    m_packageMenu->addAction(m_packageCancelAction);
    m_packageMenu->addAction(m_packageCancelDeleteAction);
    m_packageMenu->setEnabled(false);

    m_packageSubfolderAction->setCheckable(true);

    setCategoryMenuActions();

    for (int i = TransferItem::HighestPriority; i <= TransferItem::LowestPriority; i++) {
        QAction *transferAction =
        m_transferPriorityMenu->addAction(TransferItem::priorityString(TransferItem::Priority(i)),
                                          this, SLOT(setCurrentTransferPriority()));
        transferAction->setData(i);
        transferAction->setCheckable(true);
        m_transferPriorityGroup->addAction(transferAction);

        QAction *packageAction =
        m_packagePriorityMenu->addAction(TransferItem::priorityString(TransferItem::Priority(i)),
                                         this, SLOT(setCurrentPackagePriority()));
        packageAction->setData(i);
        packageAction->setCheckable(true);
        m_packagePriorityGroup->addAction(packageAction);
    }

    m_settingsAction->setShortcut(tr("Ctrl+P"));

    m_editMenu->addAction(m_settingsAction);

    m_helpMenu->addAction(m_aboutAction);

    m_optionsMenu->addMenu(m_concurrentTransfersMenu);

    m_actionSelector->setModel(new ActionModel(m_actionSelector));
    m_actionSelector->setCurrentIndex(Settings::nextAction());

    m_topToolBar->setObjectName("topToolBar");
    m_topToolBar->setWindowTitle(tr("Top toolbar"));
    m_topToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_topToolBar->setMovable(false);
    m_topToolBar->addAction(m_addUrlsAction);
    m_topToolBar->addAction(m_importUrlsAction);
    m_topToolBar->addAction(m_retrieveUrlsAction);
    m_topToolBar->addAction(m_clipboardUrlsAction);
    m_topToolBar->addSeparator();
    m_topToolBar->addAction(m_queueAction);
    m_topToolBar->addAction(m_pauseAction);
    m_topToolBar->addSeparator();
    m_topToolBar->addWidget(new QLabel(tr("After current download(s): "), m_topToolBar));
    m_topToolBar->addWidget(m_actionSelector);

    QLabel *speedIcon = new QLabel(m_bottomToolBar);
    speedIcon->setPixmap(QIcon::fromTheme("go-down").pixmap(m_bottomToolBar->iconSize()));

    QWidget *spacer = new QWidget(m_bottomToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QWidget *spacer2 = new QWidget(m_bottomToolBar);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    const int concurrent = Settings::maximumConcurrentTransfers();

    for (int i = 1; i <= MAX_CONCURRENT_TRANSFERS; i++) {
        QAction *action = m_concurrentTransfersMenu->addAction(QString::number(i), this, SLOT(setMaximumConcurrentTransfers()));
        action->setData(i);
        action->setCheckable(true);
        action->setChecked(i == concurrent);
        m_concurrentTransfersGroup->addAction(action);
    }

    m_optionsButton->setIcon(QIcon::fromTheme("document-properties"));
    m_optionsButton->setText(tr("&Options"));

    m_speedLabel->setMinimumWidth(m_speedLabel->fontMetrics().width(" 90000KB/s"));
    m_speedLabel->setAlignment(Qt::AlignCenter);

    m_bottomToolBar->setObjectName("bottomToolBar");
    m_bottomToolBar->setWindowTitle(tr("Bottom toolbar"));
    m_bottomToolBar->setAllowedAreas(Qt::BottomToolBarArea);
    m_bottomToolBar->setMovable(false);
    m_bottomToolBar->addWidget(m_optionsButton);
    m_bottomToolBar->addWidget(spacer);
    m_bottomToolBar->addWidget(m_activeLabel);
    m_bottomToolBar->addWidget(spacer2);
    m_bottomToolBar->addWidget(m_speedLabel);
    m_bottomToolBar->addWidget(speedIcon);

    m_view->setModel(TransferModel::instance());
    m_view->setItemDelegate(new TransferDelegate(m_view));
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setDragEnabled(true);
    m_view->setAcceptDrops(true);
    m_view->setDropIndicatorShown(true);
    m_view->setDragDropMode(QTreeView::InternalMove);
    m_view->setDefaultDropAction(Qt::MoveAction);
    m_view->setExpandsOnDoubleClick(true);
    m_view->setItemsExpandable(true);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);

    QHeaderView *header = m_view->header();
    const QFontMetrics fm = header->fontMetrics();
    header->resizeSection(0, 300);
    header->resizeSection(1, fm.width(TransferModel::instance()->headerData(1).toString()) + 20);
    header->resizeSection(2, fm.width("999.99MB of 999.99MB (99.99%)") + 20);
    header->resizeSection(3, fm.width("999.99KB/s") + 20);

    connect(Categories::instance(), SIGNAL(changed()), this, SLOT(setCategoryMenuActions()));

    connect(Settings::instance(), SIGNAL(nextActionChanged(int)), m_actionSelector, SLOT(setCurrentIndex(int)));
    
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
    connect(m_pluginsAction, SIGNAL(triggered()), this, SLOT(loadPlugins()));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(quit()));

    connect(m_transferPropertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentTransferProperties()));
    connect(m_transferCustomCommandAction, SIGNAL(triggered()), this, SLOT(setCurrentTransferCustomCommand()));
    connect(m_transferQueueAction, SIGNAL(triggered()), this, SLOT(queueCurrentTransfer()));
    connect(m_transferPauseAction, SIGNAL(triggered()), this, SLOT(pauseCurrentTransfer()));
    connect(m_transferCancelAction, SIGNAL(triggered()), this, SLOT(cancelCurrentTransfer()));
    connect(m_transferCancelDeleteAction, SIGNAL(triggered()), this, SLOT(cancelAndDeleteCurrentTransfer()));

    connect(m_packagePropertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentPackageProperties()));
    connect(m_packageSubfolderAction, SIGNAL(triggered(bool)), this, SLOT(setCurrentPackageSubfolder(bool)));
    connect(m_packageQueueAction, SIGNAL(triggered()), this, SLOT(queueCurrentPackage()));
    connect(m_packagePauseAction, SIGNAL(triggered()), this, SLOT(pauseCurrentPackage()));
    connect(m_packageCancelAction, SIGNAL(triggered()), this, SLOT(cancelCurrentPackage()));
    connect(m_packageCancelDeleteAction, SIGNAL(triggered()), this, SLOT(cancelAndDeleteCurrentPackage()));

    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    connect(m_actionSelector, SIGNAL(activated(int)), Settings::instance(), SLOT(setNextAction(int)));

    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentRowChanged(QModelIndex)));

    connect(m_optionsButton, SIGNAL(clicked()), this, SLOT(showOptionsMenu()));    
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Settings::setWindowGeometry(saveGeometry());
    Settings::setWindowState(saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::quit() {
    if (TransferModel::instance()->activeTransfers() > 0) {
        if (QMessageBox::question(this, tr("Quit?"),
            tr("Some downloads are still active. Do you want to quit?")) != QMessageBox::Yes) {
            return;
        }
    }

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
                                 .arg(m_view->currentIndex().data(TransferItem::NameRole).toString())) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Canceled, TransferItem::StatusRole);
        }
    }
}

void MainWindow::cancelAndDeleteCurrentTransfer() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        if (QMessageBox::question(this, tr("Delete files?"), tr("Do you want to delete the files for download '%1'?")
                                 .arg(m_view->currentIndex().data(TransferItem::NameRole).toString())) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::CanceledAndDeleted, TransferItem::StatusRole);
        }
    }
}

void MainWindow::setCurrentTransferCustomCommand() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        bool ok;
        const QString command =
        QInputDialog::getText(this, tr("Set custom command"), tr("&Custom command (%f for filename):"),
                              QLineEdit::Normal, m_view->currentIndex().data(TransferItem::CustomCommandRole)
                              .toString(), &ok);

        if (ok) {
            TransferModel::instance()->setData(m_view->currentIndex(), command, TransferItem::CustomCommandRole);
        }
    }
}

void MainWindow::setCurrentTransferPriority() {
    if (const QAction *action = m_transferPriorityGroup->checkedAction()) {
        if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
            TransferModel::instance()->setData(m_view->currentIndex(), action->data(), TransferItem::PriorityRole);
        }
    }
}

void MainWindow::showCurrentTransferProperties() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        if (TransferItem *transfer = TransferModel::instance()->get(m_view->currentIndex())) {
            TransferPropertiesDialog(transfer, this).exec();
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
                                 .arg(m_view->currentIndex().data(TransferItem::NameRole).toString())) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::Canceled, TransferItem::StatusRole);
        }
    }
}

void MainWindow::cancelAndDeleteCurrentPackage() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (QMessageBox::question(this, tr("Delete files?"), tr("Do you want to delete the files for package '%1'?")
                                 .arg(m_view->currentIndex().data(TransferItem::NameRole).toString())) == QMessageBox::Yes) {
            TransferModel::instance()->setData(m_view->currentIndex(), TransferItem::CanceledAndDeleted, TransferItem::StatusRole);
        }
    }
}

void MainWindow::setCurrentPackageCategory() {
    if (const QAction *action = m_packageCategoryGroup->checkedAction()) {
        if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
            TransferModel::instance()->setData(m_view->currentIndex(), action->data(), TransferItem::CategoryRole);
        }
    }
}

void MainWindow::setCurrentPackageSubfolder(bool enabled) {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->setData(m_view->currentIndex(), enabled, TransferItem::CreateSubfolderRole);
    }
}

void MainWindow::setCurrentPackagePriority() {
    if (const QAction *action = m_packagePriorityGroup->checkedAction()) {
        if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
            TransferModel::instance()->setData(m_view->currentIndex(), action->data(), TransferItem::PriorityRole);
        }
    }
}

void MainWindow::showCurrentPackageProperties() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (TransferItem *package = TransferModel::instance()->get(m_view->currentIndex())) {
            PackagePropertiesDialog(package, this).exec();
        }
    }
}

void MainWindow::setMaximumConcurrentTransfers() {
    if (const QAction *action = m_concurrentTransfersGroup->checkedAction()) {
        Settings::setMaximumConcurrentTransfers(action->data().toInt());
    }
}

void MainWindow::setCategoryMenuActions() {
    m_packageCategoryMenu->clear();
    QAction *categoryAction = m_packageCategoryMenu->addAction(tr("Default"), this, SLOT(setCurrentPackageCategory()));
    categoryAction->setData(QString());
    categoryAction->setCheckable(true);
    m_packageCategoryGroup->addAction(categoryAction);

    const CategoryList categories = Categories::get();

    for (int i = 0; i < categories.size(); i++) {
        categoryAction = m_packageCategoryMenu->addAction(categories.at(i).name, this, SLOT(setCurrentPackageCategory()));
        categoryAction->setData(categoryAction->text());
        categoryAction->setCheckable(true);
        m_packageCategoryGroup->addAction(categoryAction);
    }    
}

void MainWindow::setTransferMenuActions() {
    const bool canCancel = m_view->currentIndex().data(TransferItem::CanCancelRole).toBool();
    const int priority = m_view->currentIndex().data(TransferItem::PriorityRole).toInt();
    m_transferQueueAction->setEnabled(m_view->currentIndex().data(TransferItem::CanStartRole).toBool());
    m_transferPauseAction->setEnabled(m_view->currentIndex().data(TransferItem::CanPauseRole).toBool());
    m_transferCancelAction->setEnabled(canCancel);
    m_transferCancelDeleteAction->setEnabled(canCancel);

    if ((priority >= 0) && (priority < m_transferPriorityGroup->actions().size())) {
        m_transferPriorityGroup->actions().at(priority)->setChecked(true);
    }
}

void MainWindow::setPackageMenuActions() {
    const bool canCancel = m_view->currentIndex().data(TransferItem::CanCancelRole).toBool();
    const QVariant category = m_view->currentIndex().data(TransferItem::CategoryRole);
    const int priority = m_view->currentIndex().data(TransferItem::PriorityRole).toInt();
    m_packageQueueAction->setEnabled(m_view->currentIndex().data(TransferItem::CanStartRole).toBool());
    m_packagePauseAction->setEnabled(m_view->currentIndex().data(TransferItem::CanPauseRole).toBool());
    m_packageCancelAction->setEnabled(canCancel);
    m_packageCancelDeleteAction->setEnabled(canCancel);
    m_packageSubfolderAction->setChecked(m_view->currentIndex().data(TransferItem::CreateSubfolderRole).toBool());

    foreach (QAction *action, m_packageCategoryGroup->actions()) {
        if (action->data() == category) {
            action->setChecked(true);
            break;
        }
    }
    
    if ((priority >= 0) && (priority < m_packagePriorityGroup->actions().size())) {
        m_packagePriorityGroup->actions().at(priority)->setChecked(true);
    }
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

void MainWindow::showOptionsMenu() {
    m_optionsMenu->popup(m_optionsButton->mapToGlobal(m_optionsButton->rect().center()));
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
        QMessageBox::information(this, tr("Load plugins"), tr("%1 new plugins found").arg(count));
    }
    else {
        QMessageBox::information(this, tr("Load plugins"), tr("No new plugins found"));
    }
}

void MainWindow::onActiveTransfersChanged(int active) {
    m_activeLabel->setText(QString("%1DLs").arg(active));
}

void MainWindow::onCurrentRowChanged(const QModelIndex &index) {
    const int type = index.data(TransferItem::ItemTypeRole).toInt();
    m_transferMenu->setEnabled(type == TransferItem::TransferType);
    m_packageMenu->setEnabled(type == TransferItem::PackageType);
}

void MainWindow::onMaximumConcurrentTransfersChanged(int maximum) {
    foreach (QAction *action, m_concurrentTransfersGroup->actions()) {
        if (action->data() == maximum) {
            action->setChecked(true);
            return;
        }
    }
}

void MainWindow::onTotalSpeedChanged(int speed) {
    m_speedLabel->setText(Utils::formatBytes(speed) + "/s");
}
