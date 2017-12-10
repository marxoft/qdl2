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
#include "categories.h"
#include "clipboardurlsdialog.h"
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
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_fileMenu(new QMenu(tr("&File"), this)),
    m_transferMenu(new QMenu(tr("&Download"), this)),
    m_packageMenu(new QMenu(tr("&Package"), this)),
    m_viewMenu(new QMenu(tr("&View"), this)),
    m_toolsMenu(new QMenu(tr("&Tools"), this)),
    m_helpMenu(new QMenu(tr("&Help"), this)),
    m_transferPriorityMenu(new QMenu(tr("&Priority"), this)),
    m_packageCategoryMenu(new QMenu(tr("&Category"), this)),
    m_packagePriorityMenu(new QMenu(tr("&Priority"), this)),
    m_topToolBar(new QToolBar(this)),
    m_bottomToolBar(new QToolBar(this)),
    m_addUrlsAction(new QAction(QIcon::fromTheme("list-add"), tr("&Add URLs"), this)),
    m_importUrlsAction(new QAction(QIcon::fromTheme("document-open"), tr("&Import URLs"), this)),
    m_retrieveUrlsAction(new QAction(QIcon::fromTheme("folder-remote"), tr("&Retrieve URLs"), this)),
    m_clipboardUrlsAction(new QAction(QIcon::fromTheme("edit-paste"), tr("Show &clipboard URLs"), this)),
    m_downloadRequestAction(new QAction(QIcon::fromTheme("download"), tr("Retrieve &download requests"), this)),
    m_searchAction(new QAction(QIcon::fromTheme("edit-find"), tr("&Search for content"), this)),
    m_queueAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("&Start all downloads"), this)),
    m_pauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause all downloads"), this)),
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
    m_closePageAction(new QAction(QIcon::fromTheme("view-close"), tr("&Close tab"), this)),
    m_transfersAction(new QAction(tr("Downloads"), this)),
    m_pluginsAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Load plugins"), this)),
    m_settingsAction(new QAction(QIcon::fromTheme("preferences-desktop"), tr("&Preferences"), this)),
    m_aboutAction(new QAction(QIcon::fromTheme("help-info"), tr("&About"), this)),
    m_transferPriorityGroup(new QActionGroup(this)),
    m_packageCategoryGroup(new QActionGroup(this)),
    m_packagePriorityGroup(new QActionGroup(this)),
    m_pageGroup(new QActionGroup(this)),
    m_concurrentTransfersSpinBox(new QSpinBox(this)),
    m_actionSelector(new QComboBox(this)),
    m_messageLabel(new QLabel(this)),
    m_activeLabel(new QLabel(QString("%1DLs").arg(TransferModel::instance()->activeTransfers()), this)),
    m_speedLabel(new QLabel(Utils::formatBytes(TransferModel::instance()->totalSpeed()) + "/s", this)),
    m_widget(new QWidget(this)),
    m_tabs(new QTabBar(m_widget)),
    m_stack(new QStackedWidget(m_widget)),
    m_view(new QTreeView(m_stack)),
    m_layout(new QVBoxLayout(m_widget))
{
    setWindowTitle("QDL");
    setCentralWidget(m_widget);
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
    menuBar()->addMenu(m_viewMenu);
    menuBar()->addMenu(m_toolsMenu);
    menuBar()->addMenu(m_helpMenu);

    m_addUrlsAction->setShortcut(tr("Ctrl+N"));
    m_addUrlsAction->setToolTip(tr("Add URLs to downloads"));
    m_importUrlsAction->setShortcut(tr("Ctrl+O"));
    m_importUrlsAction->setToolTip(tr("Import URLs from a text file"));
    m_retrieveUrlsAction->setShortcut(tr("Ctrl+F"));
    m_retrieveUrlsAction->setToolTip(tr("Retrieve supported URLs from a web page"));
    m_clipboardUrlsAction->setShortcut(tr("Ctrl+U"));
    m_clipboardUrlsAction->setToolTip(tr("Show URLs retrieved from the clipboard"));
    m_downloadRequestAction->setShortcut(tr("Ctrl+D"));
    m_downloadRequestAction->setToolTip(tr("Retrieve download request metadata from URLs"));
    m_searchAction->setShortcut(tr("Ctrl+S"));
    m_searchAction->setToolTip(tr("Search for content using a plugin"));
    m_searchAction->setEnabled(SearchPluginManager::instance()->count() > 0);
    m_quitAction->setShortcut(tr("Ctrl+Q"));
    m_quitAction->setToolTip(tr("Quit the application"));

    m_fileMenu->addAction(m_addUrlsAction);
    m_fileMenu->addAction(m_importUrlsAction);
    m_fileMenu->addAction(m_retrieveUrlsAction);
    m_fileMenu->addAction(m_clipboardUrlsAction);
    m_fileMenu->addAction(m_downloadRequestAction);
    m_fileMenu->addAction(m_searchAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_queueAction);
    m_fileMenu->addAction(m_pauseAction);
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
            m_transferPriorityMenu->addAction(TransferItem::priorityString(TransferItem::Priority(i)));
        transferAction->setData(i);
        transferAction->setCheckable(true);
        m_transferPriorityGroup->addAction(transferAction);

        QAction *packageAction =
            m_packagePriorityMenu->addAction(TransferItem::priorityString(TransferItem::Priority(i)));
        packageAction->setData(i);
        packageAction->setCheckable(true);
        m_packagePriorityGroup->addAction(packageAction);
    }
    
    m_closePageAction->setShortcut(tr("Ctrl+W"));
    m_closePageAction->setEnabled(false);
    m_transfersAction->setCheckable(true);
    m_transfersAction->setChecked(true);
    
    m_pageGroup->setVisible(false);
    m_pageGroup->addAction(m_transfersAction);

    m_viewMenu->addAction(m_closePageAction);
    m_viewMenu->addAction(m_transfersAction);

    m_pluginsAction->setShortcut(tr("Ctrl+L"));
    m_settingsAction->setShortcut(tr("Ctrl+P"));

    m_toolsMenu->addAction(m_pluginsAction);
    m_toolsMenu->addAction(m_settingsAction);

    m_helpMenu->addAction(m_aboutAction);

    m_concurrentTransfersSpinBox->setRange(1, MAX_CONCURRENT_TRANSFERS);
    m_concurrentTransfersSpinBox->setValue(Settings::maximumConcurrentTransfers());

    QLabel *concurrentLabel = new QLabel(tr("&Maximum concurrent downloads: "), this);
    concurrentLabel->setBuddy(m_concurrentTransfersSpinBox);

    m_actionSelector->setModel(new ActionModel(m_actionSelector));
    m_actionSelector->setCurrentIndex(Settings::nextAction());
    
    QLabel *actionLabel = new QLabel(tr(" &After current download(s): "), this);
    actionLabel->setBuddy(m_actionSelector);

    m_topToolBar->setObjectName("topToolBar");
    m_topToolBar->setWindowTitle(tr("Top toolbar"));
    m_topToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_topToolBar->setMovable(false);
    m_topToolBar->addAction(m_addUrlsAction);
    m_topToolBar->addAction(m_importUrlsAction);
    m_topToolBar->addAction(m_retrieveUrlsAction);
    m_topToolBar->addAction(m_clipboardUrlsAction);
    m_topToolBar->addAction(m_downloadRequestAction);
    m_topToolBar->addAction(m_searchAction);
    m_topToolBar->addSeparator();
    m_topToolBar->addAction(m_queueAction);
    m_topToolBar->addAction(m_pauseAction);
    m_topToolBar->addSeparator();
    m_topToolBar->addWidget(concurrentLabel);
    m_topToolBar->addWidget(m_concurrentTransfersSpinBox);
    m_topToolBar->addWidget(actionLabel);
    m_topToolBar->addWidget(m_actionSelector);

    QLabel *speedIcon = new QLabel(m_bottomToolBar);
    speedIcon->setPixmap(QIcon::fromTheme("go-down").pixmap(m_bottomToolBar->iconSize()));

    QWidget *spacer = new QWidget(m_bottomToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_messageLabel->setMargin(6);
    m_activeLabel->setMargin(6);
    m_speedLabel->setMargin(6);

    m_bottomToolBar->setObjectName("bottomToolBar");
    m_bottomToolBar->setWindowTitle(tr("Bottom toolbar"));
    m_bottomToolBar->setAllowedAreas(Qt::BottomToolBarArea);
    m_bottomToolBar->setMovable(false);
    m_bottomToolBar->addWidget(m_messageLabel);
    m_bottomToolBar->addWidget(spacer);
    m_bottomToolBar->addWidget(m_speedLabel);
    m_bottomToolBar->addSeparator();
    m_bottomToolBar->addWidget(m_activeLabel);
    m_bottomToolBar->addWidget(speedIcon);
    
    m_tabs->setTabsClosable(true);
    m_tabs->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    m_tabs->setExpanding(false);
    m_tabs->addTab(tr("Downloads"));
    m_tabs->hide();
    
    if (QWidget *button = m_tabs->tabButton(0, QTabBar::RightSide)) {
        button->hide();
    }

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
    
    if (!header->restoreState(Settings::transferViewHeaderState())) {
        const QFontMetrics fm = header->fontMetrics();
        header->resizeSection(0, 300);
        header->resizeSection(1, fm.width(TransferModel::instance()->headerData(1).toString()) + 20);
        header->resizeSection(2, fm.width("999.99MB of 999.99MB (99.99%)") + 20);
        header->resizeSection(3, fm.width("999.99KB/s") + 20);
    }
    
    m_stack->addWidget(m_view);
    
    m_layout->addWidget(m_tabs);
    m_layout->addWidget(m_stack);
    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(Categories::instance(), SIGNAL(changed()), this, SLOT(setCategoryMenuActions()));

    connect(Settings::instance(), SIGNAL(nextActionChanged(int)), m_actionSelector, SLOT(setCurrentIndex(int)));
    connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int)),
            m_concurrentTransfersSpinBox, SLOT(setValue(int)));
    
    connect(TransferModel::instance(), SIGNAL(captchaRequest(TransferItem*)), this, SLOT(showCaptchaDialog(TransferItem*)));
    connect(TransferModel::instance(), SIGNAL(settingsRequest(TransferItem*)), this, SLOT(showPluginSettingsDialog(TransferItem*)));
    connect(TransferModel::instance(), SIGNAL(activeTransfersChanged(int)), this, SLOT(onActiveTransfersChanged(int)));
    connect(TransferModel::instance(), SIGNAL(totalSpeedChanged(int)), this, SLOT(onTotalSpeedChanged(int)));

    connect(m_transferMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    connect(m_packageMenu, SIGNAL(aboutToShow()), this, SLOT(setPackageMenuActions()));

    connect(m_addUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    connect(m_importUrlsAction, SIGNAL(triggered()), this, SLOT(showImportUrlsDialog()));
    connect(m_retrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    connect(m_clipboardUrlsAction, SIGNAL(triggered()), this, SLOT(showClipboardUrlsDialog()));
    connect(m_downloadRequestAction, SIGNAL(triggered()), this, SLOT(showDownloadRequestDialog()));
    connect(m_searchAction, SIGNAL(triggered()), this, SLOT(showSearchDialog()));
    connect(m_queueAction, SIGNAL(triggered()), TransferModel::instance(), SLOT(queue()));
    connect(m_pauseAction, SIGNAL(triggered()), TransferModel::instance(), SLOT(pause()));
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
    
    connect(m_closePageAction, SIGNAL(triggered()), this, SLOT(closeCurrentPage()));
    
    connect(m_pluginsAction, SIGNAL(triggered()), this, SLOT(loadPlugins()));
    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    
    connect(m_transferPriorityGroup, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentTransferPriority(QAction*)));
    connect(m_packageCategoryGroup, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentPackageCategory(QAction*)));
    connect(m_packagePriorityGroup, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentPackagePriority(QAction*)));
    connect(m_pageGroup, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentPage(QAction*)));

    connect(m_concurrentTransfersSpinBox, SIGNAL(valueChanged(int)),
            Settings::instance(), SLOT(setMaximumConcurrentTransfers(int)));
    connect(m_actionSelector, SIGNAL(activated(int)), Settings::instance(), SLOT(setNextAction(int)));
        
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(setCurrentPage(int)));
    connect(m_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closePage(int)));

    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentRowChanged(QModelIndex)));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Settings::setTransferViewHeaderState(m_view->header()->saveState());
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

void MainWindow::setCurrentTransferPriority(QAction *action) {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
        TransferModel::instance()->setData(m_view->currentIndex(), action->data(), TransferItem::PriorityRole);
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

void MainWindow::setCurrentPackageCategory(QAction *action) {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->setData(m_view->currentIndex(), action->data(), TransferItem::CategoryRole);
    }
}

void MainWindow::setCurrentPackageSubfolder(bool enabled) {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->setData(m_view->currentIndex(), enabled, TransferItem::CreateSubfolderRole);
    }
}

void MainWindow::setCurrentPackagePriority(QAction *action) {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        TransferModel::instance()->setData(m_view->currentIndex(), action->data(), TransferItem::PriorityRole);
    }
}

void MainWindow::showCurrentPackageProperties() {
    if (m_view->currentIndex().data(TransferItem::ItemTypeRole) == TransferItem::PackageType) {
        if (TransferItem *package = TransferModel::instance()->get(m_view->currentIndex())) {
            PackagePropertiesDialog(package, this).exec();
        }
    }
}

void MainWindow::setCategoryMenuActions() {
    m_packageCategoryMenu->clear();
    QAction *categoryAction = m_packageCategoryMenu->addAction(tr("Default"));
    categoryAction->setData(QString());
    categoryAction->setCheckable(true);
    m_packageCategoryGroup->addAction(categoryAction);

    const CategoryList categories = Categories::get();

    for (int i = 0; i < categories.size(); i++) {
        categoryAction = m_packageCategoryMenu->addAction(categories.at(i).name);
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
        m_packageMenu->popup(m_view->mapToGlobal(pos), m_packagePropertiesAction);
    }
    else {
        m_transferMenu->popup(m_view->mapToGlobal(pos), m_transferPropertiesAction);
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
                m_pageGroup->setVisible(false);
            }
            
            if ((index >= 0) && (index < m_pageGroup->actions().size())) {
                QAction *action = m_pageGroup->actions().at(index);
                m_pageGroup->removeAction(action);
                m_viewMenu->removeAction(action);
                action->deleteLater();
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
    
    if ((index >= 0) && (index < m_pageGroup->actions().size())) {
        m_pageGroup->actions().at(index)->setChecked(true);
    }

    if (const Page *page = qobject_cast<Page*>(m_stack->currentWidget())) {
        showMessage(page->statusString());
    }
    else {
        showMessage(QString());
    }
}

void MainWindow::setCurrentPage(QAction *action) {
    m_tabs->setCurrentIndex(m_pageGroup->actions().indexOf(action));

    if (const Page *page = qobject_cast<Page*>(m_stack->currentWidget())) {
        showMessage(page->statusString());
    }
    else {
        showMessage(QString());
    }
}

void MainWindow::search(const QString &pluginName, const QString &pluginId) {
    const QString text = tr("Search - %1").arg(pluginName);
    SearchPage *page = new SearchPage(m_stack);
    page->setAttribute(Qt::WA_DeleteOnClose, true);
    m_stack->addWidget(page);
    const int index = m_stack->indexOf(page);
    m_tabs->addTab(text);
    m_stack->setCurrentIndex(index);
    m_tabs->setCurrentIndex(index);
    m_tabs->show();
    QAction *action = m_viewMenu->addAction(text);
    m_pageGroup->addAction(action);
    m_pageGroup->setVisible(true);
    action->setCheckable(true);
    action->setChecked(true);
    connect(page, SIGNAL(statusChanged(Page::Status)), this, SLOT(onPageStatusChanged()));
    page->search(pluginId);
}

void MainWindow::showAddUrlsDialog() {
    AddUrlsDialog addDialog(this);

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

void MainWindow::showAddUrlsDialog(const QStringList &urls) {
    AddUrlsDialog addDialog(this);
    addDialog.setUrls(urls);

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
}

void MainWindow::showRetrieveUrlsDialog() {
    RetrieveUrlsDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList results = dialog.results();
        dialog.clear();

        if (!results.isEmpty()) {
            TextEditDialog dialog(results.join("\n"), this);
            dialog.setWindowTitle(tr("Retrieve URLs"));
            dialog.setLabelText(tr("Results:"));
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
            dialog.setLabelText(tr("Results:"));
            dialog.exec();
        }
    }
}

void MainWindow::showClipboardUrlsDialog() {
    ClipboardUrlsDialog(this).exec();
}

void MainWindow::showDownloadRequestDialog() {
    const QStringList urls = TextEditDialog::getText(this, tr("Retrieve download requests"), tr("URLs:"))
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
        dialog.setLabelText(tr("Results:"));
        dialog.exec();
    }
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
        showMessage(tr("%1 new plugins found").arg(count));
        
        if (search > 0) {
            m_searchAction->setEnabled(true);
        }
    }
    else {
        showMessage(tr("No new plugins found"));
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

void MainWindow::onPageStatusChanged() {
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

void MainWindow::onTotalSpeedChanged(int speed) {
    m_speedLabel->setText(Utils::formatBytes(speed) + "/s");
}
