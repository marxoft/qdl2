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

#ifndef CATEGORIESDIALOG_H
#define CATEGORIESDIALOG_H

#include <QDialog>

class CategoryModel;
class QDialogButtonBox;
class QGridLayout;
class QLineEdit;
class QMaemo5ValueButton;
class QModelIndex;
class QTreeView;

class CategoriesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoriesDialog(QWidget *parent = 0);

private Q_SLOTS:
    void addCategory();
    void setCurrentCategory(const QModelIndex &index);
    
    void showContextMenu(const QPoint &pos);
    void showFileDialog();

    void onNameChanged(const QString &name);

private:
    CategoryModel *m_model;
    
    QTreeView *m_view;

    QLineEdit *m_nameEdit;

    QMaemo5ValueButton *m_pathButton;
    
    QDialogButtonBox *m_buttonBox;

    QGridLayout *m_layout;

    QString m_path;
};

#endif // CATEGORIESDIALOG_H
