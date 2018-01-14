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
class ValueSelectorAction;
class QLabel;
class QModelIndex;
class QStackedWidget;
class QTabBar;
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
    void queueCurrentTransfer();
    void pauseCurrentTransfer();
    void cancelCurrentTransfer();
    void cancelAndDeleteCurrentTransfer();

    void queueCurrentPackage();
    void pauseCurrentPackage();
    void cancelCurrentPackage();
    void cancelAndDeleteCurrentPackage();
    
    void showCurrentItemProperties();

    void setMaximumConcurrentTransfers(const QVariant &maximum);
    void setNextAction(const QVariant &action);

    void setTransferMenuActions();
    void setPackageMenuActions();

    void showContextMenu(const QPoint &pos);
    
    void closePage(int index);
    void closeCurrentPage();
    void setCurrentPage(int index);
    void showFirstPage();
    void showLastPage();
    void showNextPage();
    void showPreviousPage();
    
    void search(const QString &pluginName, const QString &pluginId);
    
    void showSearchDialog();
    void showSettingsDialog();
    void showAboutDialog();
    void showAddUrlsDialog();
    void showAddUrlsDialog(const QStringList &urls);
    void showImportUrlsDialog();
    void showRetrieveUrlsDialog();
    void showRetrieveUrlsDialog(const QStringList &urls);
    void showClipboardUrlsDialog();
    void showDownloadRequestDialog();
    void showDownloadRequestDialog(const QStringList &urls);
    
    void showCaptchaDialog(TransferItem *transfer);
    void showPluginSettingsDialog(TransferItem *transfer);

    void showMessage(const QString &message);
    void showError(const QString &errorString);

    void loadPlugins();

    void onActiveTransfersChanged(int active);
    void onCurrentRowChanged(const QModelIndex &index);
    void onMaximumConcurrentTransfersChanged(int maximum);
    void onNextActionChanged(int action);
    void onPageStatusChanged();
    void onTotalSpeedChanged(int speed);
    
private:
    QMenu *m_transferMenu;
    QMenu *m_packageMenu;
    
    ValueSelectorAction *m_concurrentAction;
    ValueSelectorAction *m_nextAction;
    QAction *m_searchAction;
    QAction *m_settingsAction;
    QAction *m_pluginsAction;
    QAction *m_aboutAction;
    
    QAction *m_closePageAction;
    QAction *m_firstPageAction;
    QAction *m_lastPageAction;
    QAction *m_nextPageAction;
    QAction *m_previousPageAction;
    QAction *m_scrollBottomAction;
    QAction *m_scrollTopAction;
    
    QAction *m_addUrlsAction;
    QAction *m_importUrlsAction;
    QAction *m_retrieveUrlsAction;
    QAction *m_clipboardUrlsAction;
    QAction *m_downloadRequestAction;
    QAction *m_queueAction;
    QAction *m_pauseAction;
    QAction *m_propertiesAction;
        
    QAction *m_transferQueueAction;
    QAction *m_transferPauseAction;
    QAction *m_transferCancelAction;
    QAction *m_transferCancelDeleteAction;

    QAction *m_packageQueueAction;
    QAction *m_packagePauseAction;
    QAction *m_packageCancelAction;
    QAction *m_packageCancelDeleteAction;
    
    QWidget *m_widget;
    
    QTabBar *m_tabs;
    
    QStackedWidget *m_stack;
    
    QTreeView *m_view;
    
    QToolBar *m_toolBar;
    
    QLabel *m_messageLabel;
    
    QVBoxLayout *m_layout;
};

#endif // MAINWINDOW_H
