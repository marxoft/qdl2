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

#include "categoriesdialog.h"
#include "categorymodel.h"
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMaemo5ValueButton>
#include <QMenu>
#include <QTreeView>

CategoriesDialog::CategoriesDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new CategoryModel(this)),
    m_view(new QTreeView(this)),
    m_nameEdit(new QLineEdit(this)),
    m_pathButton(new QMaemo5ValueButton(tr("Download path"), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("Categories"));
    setMinimumHeight(360);

    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setItemsExpandable(false);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setRootIsDecorated(false);
    m_view->header()->setStretchLastSection(true);

    m_nameEdit->setPlaceholderText(tr("Name"));

    m_pathButton->setValueText(tr("None chosen"));

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_layout->addWidget(m_view, 0, 0);
    m_layout->addWidget(m_nameEdit, 1, 0);
    m_layout->addWidget(m_pathButton, 2, 0);
    m_layout->addWidget(m_buttonBox, 2, 1);
    m_layout->setRowStretch(0, 1);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentCategory(QModelIndex)));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onNameChanged(QString)));
    connect(m_nameEdit, SIGNAL(returnPressed()), m_buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));
    connect(m_pathButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(addCategory()));
}

void CategoriesDialog::addCategory() {
    m_model->append(m_nameEdit->text(), m_path);
    m_nameEdit->clear();
}

void CategoriesDialog::setCurrentCategory(const QModelIndex &index) {
    m_path = index.data(CategoryModel::ValueRole).toString();
    m_nameEdit->setText(index.data(CategoryModel::NameRole).toString());
    m_pathButton->setValueText(m_path);
}

void CategoriesDialog::showContextMenu(const QPoint &pos) {
    if (!m_view->currentIndex().isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction(tr("Remove"));

    if (menu.exec(mapToGlobal(pos))) {
        m_model->remove(m_view->currentIndex().row());
    }
}

void CategoriesDialog::showFileDialog() {
    const QString path = QFileDialog::getExistingDirectory(this, tr("Path"), m_path);

    if (!path.isEmpty()) {
        m_path = path;
        m_pathButton->setValueText(path);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!m_nameEdit->text().isEmpty());
    }
}

void CategoriesDialog::onNameChanged(const QString &name) {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled((!name.isEmpty()) && (!m_path.isEmpty()));
}
