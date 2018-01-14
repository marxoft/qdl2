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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TransferItem;
class QActionGroup;
class QComboBox;
class QLabel;
class QSpinBox;
class QTreeView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    virtual QSize sizeHint() const;

protected:
    virtual void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void reloadCurrentTransfer();
    void queueCurrentTransfer();
    void pauseCurrentTransfer();
    void cancelCurrentTransfer();
    void cancelAndDeleteCurrentTransfer();
    void setCurrentTransferCustomCommand();
    void setCurrentTransferPriority(QAction *action);
    void showCurrentTransferProperties();

    void reloadCurrentPackage();
    void queueCurrentPackage();
    void pauseCurrentPackage();
    void cancelCurrentPackage();
    void cancelAndDeleteCurrentPackage();
    void setCurrentPackageSubfolder(bool enabled);
    void setCurrentPackageCategory(QAction *action);
    void setCurrentPackagePriority(QAction *action);
    void showCurrentPackageProperties();

    void setCategoryMenuActions();
    void setTransferMenuActions();
    void setPackageMenuActions();

    void showContextMenu(const QPoint &pos);
    
    void showAddUrlsDialog();
    void showAddUrlsDialog(const QStringList &urls);
    void showImportUrlsDialog();
    void showRetrieveUrlsDialog();
    void showRetrieveUrlsDialog(const QStringList &urls);
    void showClipboardUrlsDialog();
    void showDownloadRequestDialog();
    void showDownloadRequestDialog(const QStringList &urls);
    void showSettingsDialog();
    void showAboutDialog();
    
    void showCaptchaDialog(TransferItem *transfer);
    void showPluginSettingsDialog(TransferItem *transfer);

    void showMessage(const QString &message);
    void showError(const QString &errorString);
    
    void reload();

    void onActiveTransfersChanged(int active);
    void onCurrentRowChanged(const QModelIndex &index);
    void onTotalSpeedChanged(int speed);
    
private:
    QMenu *m_fileMenu;
    QMenu *m_transferMenu;
    QMenu *m_packageMenu;
    QMenu *m_toolsMenu;
    QMenu *m_helpMenu;

    QMenu *m_transferPriorityMenu;

    QMenu *m_packageCategoryMenu;
    QMenu *m_packagePriorityMenu;

    QToolBar *m_topToolBar;
    QToolBar *m_bottomToolBar;

    QAction *m_addUrlsAction;
    QAction *m_importUrlsAction;
    QAction *m_retrieveUrlsAction;
    QAction *m_clipboardUrlsAction;
    QAction *m_downloadRequestAction;
    QAction *m_queueAction;
    QAction *m_pauseAction;
    QAction *m_quitAction;

    QAction *m_transferReloadAction;
    QAction *m_transferPropertiesAction;
    QAction *m_transferCustomCommandAction;
    QAction *m_transferQueueAction;
    QAction *m_transferPauseAction;
    QAction *m_transferCancelAction;
    QAction *m_transferCancelDeleteAction;

    QAction *m_packageReloadAction;
    QAction *m_packagePropertiesAction;
    QAction *m_packageSubfolderAction;
    QAction *m_packageQueueAction;
    QAction *m_packagePauseAction;
    QAction *m_packageCancelAction;
    QAction *m_packageCancelDeleteAction;
    
    QAction *m_transfersAction;
    
    QAction *m_autoReloadAction;
    QAction *m_reloadAction;
    QAction *m_settingsAction;

    QAction *m_aboutAction;

    QActionGroup *m_transferPriorityGroup;
    QActionGroup *m_packageCategoryGroup;
    QActionGroup *m_packagePriorityGroup;

    QSpinBox *m_concurrentTransfersSpinBox;

    QComboBox *m_actionSelector;
    
    QLabel *m_messageLabel;
    QLabel *m_activeLabel;
    QLabel *m_speedLabel;

    QTreeView *m_view;
};

#endif // MAINWINDOW_H
