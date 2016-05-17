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

#include "archivepasswordsdialog.h"
#include "archivepasswordmodel.h"
#include <QAction>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>

ArchivePasswordsDialog::ArchivePasswordsDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new ArchivePasswordModel(this)),
    m_view(new QListView(this)),
    m_edit(new QLineEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("Archive passwords"));
    setMinimumHeight(360);

    m_view->setModel(m_model);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    m_edit->setPlaceholderText(tr("Add password"));

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_layout->addWidget(m_view, 0, 0);
    m_layout->addWidget(m_edit, 1, 0);
    m_layout->addWidget(m_buttonBox, 1, 1);
    m_layout->setRowStretch(0, 1);

    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_edit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordChanged(QString)));
    connect(m_edit, SIGNAL(returnPressed()), m_buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(addPassword()));
}

void ArchivePasswordsDialog::showContextMenu(const QPoint &pos) {
    if (!m_view->currentIndex().isValid()) {
        return;
    }
    
    QMenu menu(this);
    menu.addAction(tr("Remove"));
    
    if (menu.exec(m_view->mapToGlobal(pos))) {
        m_model->remove(m_view->currentIndex().row());
    }
}

void ArchivePasswordsDialog::addPassword() {
    m_model->append(m_edit->text());
    m_edit->clear();
}

void ArchivePasswordsDialog::onPasswordChanged(const QString &password) {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!password.isEmpty());
}
