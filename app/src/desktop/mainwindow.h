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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TransferItem;
class QActionGroup;
class QComboBox;
class QLabel;
class QStackedWidget;
class QTabBar;
class QToolButton;
class QTreeView;
class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void quit();
    
    void queueCurrentTransfer();
    void pauseCurrentTransfer();
    void cancelCurrentTransfer();
    void cancelAndDeleteCurrentTransfer();
    void setCurrentTransferCustomCommand();
    void setCurrentTransferPriority();
    void showCurrentTransferProperties();

    void queueCurrentPackage();
    void pauseCurrentPackage();
    void cancelCurrentPackage();
    void cancelAndDeleteCurrentPackage();
    void setCurrentPackageSubfolder(bool enabled);
    void setCurrentPackageCategory();
    void setCurrentPackagePriority();
    void showCurrentPackageProperties();

    void setMaximumConcurrentTransfers();

    void setCategoryMenuActions();
    void setTransferMenuActions();
    void setPackageMenuActions();

    void showContextMenu(const QPoint &pos);
    void showOptionsMenu();
    
    void closePage(int index);
    void closeCurrentPage();
    void setCurrentPage(int index);
    
    void search(const QString &query, const QString &pluginName, const QString &pluginId);
    
    void showAddUrlsDialog();
    void showAddUrlsDialog(const QStringList &urls);
    void showImportUrlsDialog();
    void showRetrieveUrlsDialog();
    void showRetrieveUrlsDialog(const QStringList &urls);
    void showClipboardUrlsDialog();
    void showSearchDialog();
    void showSettingsDialog();
    void showAboutDialog();
    
    void showCaptchaDialog(TransferItem *transfer);
    void showPluginSettingsDialog(TransferItem *transfer);
    
    void loadPlugins();

    void onActiveTransfersChanged(int active);
    void onCurrentRowChanged(const QModelIndex &index);
    void onMaximumConcurrentTransfersChanged(int maximum);
    void onTotalSpeedChanged(int speed);
    
private:    
    QMenu *m_fileMenu;
    QMenu *m_transferMenu;
    QMenu *m_packageMenu;
    QMenu *m_editMenu;
    QMenu *m_helpMenu;

    QMenu *m_transferPriorityMenu;

    QMenu *m_packageCategoryMenu;
    QMenu *m_packagePriorityMenu;

    QMenu *m_optionsMenu;
    
    QMenu *m_concurrentTransfersMenu;

    QToolBar *m_topToolBar;
    QToolBar *m_bottomToolBar;

    QAction *m_addUrlsAction;
    QAction *m_importUrlsAction;
    QAction *m_retrieveUrlsAction;
    QAction *m_clipboardUrlsAction;
    QAction *m_searchAction;
    QAction *m_queueAction;
    QAction *m_pauseAction;
    QAction *m_pluginsAction;
    QAction *m_quitAction;

    QAction *m_transferPropertiesAction;
    QAction *m_transferCustomCommandAction;
    QAction *m_transferQueueAction;
    QAction *m_transferPauseAction;
    QAction *m_transferCancelAction;
    QAction *m_transferCancelDeleteAction;

    QAction *m_packagePropertiesAction;
    QAction *m_packageSubfolderAction;
    QAction *m_packageQueueAction;
    QAction *m_packagePauseAction;
    QAction *m_packageCancelAction;
    QAction *m_packageCancelDeleteAction;

    QAction *m_settingsAction;

    QAction *m_aboutAction;

    QActionGroup *m_transferPriorityGroup;
    QActionGroup *m_packageCategoryGroup;
    QActionGroup *m_packagePriorityGroup;
    QActionGroup *m_concurrentTransfersGroup;

    QComboBox *m_actionSelector;
    
    QLabel *m_activeLabel;
    QLabel *m_speedLabel;
    
    QToolButton *m_optionsButton;
    
    QWidget *m_widget;
    
    QTabBar *m_tabs;
    
    QStackedWidget *m_stack;

    QTreeView *m_view;
    
    QVBoxLayout *m_layout;
};

#endif // MAINWINDOW_H
