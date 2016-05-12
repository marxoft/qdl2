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

#include "clipboardurlsdialog.h"
#include "clipboardurlmodel.h"
#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QListView>
#include <QMenu>
#include <QPushButton>

static bool rowLessThan(const QModelIndex &index, const QModelIndex &other) {
    return index.row() < other.row();
}

ClipboardUrlsDialog::ClipboardUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_actionModel(new UrlActionModel(this)),
    m_view(new QListView(this)),
    m_actionSelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Clipboard URLs"));

    m_view->setModel(ClipboardUrlModel::instance());
    m_view->setSelectionMode(QListView::MultiSelection);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    m_actionSelector->setModel(m_actionModel);

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_layout->addRow(m_view);
    m_layout->addRow(tr("&Action:"), m_actionSelector);
    m_layout->addRow(m_buttonBox);

    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(onSelectionChanged()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

Qdl::UrlAction ClipboardUrlsDialog::action() const {
    return Qdl::UrlAction(m_actionSelector->itemData(m_actionSelector->currentIndex()).toInt());
}

void ClipboardUrlsDialog::setAction(Qdl::UrlAction action) {
    m_actionSelector->setCurrentIndex(m_actionSelector->findData(action));
}

QStringList ClipboardUrlsDialog::urls() const {
    if (!m_view->selectionModel()->hasSelection()) {
        return QStringList();
    }
    
    QStringList list;
    QModelIndexList rows = m_view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), rowLessThan);

    for (int i = 0; i < rows.size(); i++) {
        list << rows.at(i).data().toString();
    }

    return list;
}

void ClipboardUrlsDialog::showContextMenu(const QPoint &pos) {
    if (!m_view->selectionModel()->hasSelection()) {
        return;
    }

    QMenu menu(this);
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("Remove"));

    if (menu.exec(m_view->mapToGlobal(pos))) {
        QModelIndexList rows = m_view->selectionModel()->selectedRows();
        qSort(rows.begin(), rows.end(), rowLessThan);

        for (int i = rows.size() - 1; i >= 0; i--) {
            ClipboardUrlModel::instance()->remove(rows.at(i).row());
        }
    }
}

void ClipboardUrlsDialog::onSelectionChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_view->selectionModel()->hasSelection());
}
