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

#ifndef ARCHIVEPASSWORDSDIALOG_H
#define ARCHIVEPASSWORDSDIALOG_H

#include <QDialog>

class ArchivePasswordModel;
class QDialogButtonBox;
class QGridLayout;
class QLineEdit;
class QListView;

class ArchivePasswordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArchivePasswordsDialog(QWidget *parent = 0);

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);

    void addPassword();

    void onPasswordChanged(const QString &password);

private:
    ArchivePasswordModel *m_model;

    QListView *m_view;

    QLineEdit *m_edit;

    QDialogButtonBox *m_buttonBox;

    QGridLayout *m_layout;
};
    
#endif // ARCHIVEPASSWORDSDIALOG_H
