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

#ifndef PACKAGEPROPERTIESDIALOG_H
#define PACKAGEPROPERTIESDIALOG_H

#include "transferitem.h"
#include <QDialog>
#include <QPointer>

class CategorySelectionModel;
class TransferItemPriorityModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QLabel;
class QProgressBar;

class PackagePropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PackagePropertiesDialog(TransferItem *package, QWidget *parent = 0);

private Q_SLOTS:
    void setCategory(int index);
    void setCreateSubfolder(bool enabled);
    void setPriority(int index);
    
    void onDataChanged(TransferItem *package, int role);

private:
    void updateCategory(TransferItem *package);
    void updateCreateSubfolder(TransferItem *package);
    void updateName(TransferItem *package);
    void updatePriority(TransferItem *package);
    void updateProgress(TransferItem *package);
    void updateStatus(TransferItem *package);
    
    QPointer<TransferItem> m_package;

    CategorySelectionModel *m_categoryModel;
    TransferItemPriorityModel *m_priorityModel;

    QLabel *m_nameLabel;
    QLabel *m_statusLabel;

    QCheckBox *m_subfolderCheckBox;

    QComboBox *m_categorySelector;
    QComboBox *m_prioritySelector;

    QProgressBar *m_progressBar;

    QDialogButtonBox *m_buttonBox;

    QFormLayout *m_layout;
};
    
#endif // PACKAGEPROPERTIESDIALOG_H
