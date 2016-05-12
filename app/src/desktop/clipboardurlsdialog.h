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

#ifndef CLIPBOARDURLSDIALOG_H
#define CLIPBOARDURLSDIALOG_H

#include "urlactionmodel.h"
#include <QDialog>

class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QListView;

class ClipboardUrlsDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(Qdl::UrlAction action READ action WRITE setAction)
    Q_PROPERTY(QStringList urls READ urls)

public:
    explicit ClipboardUrlsDialog(QWidget *parent = 0);

    Qdl::UrlAction action() const;
    
    QStringList urls() const;

public Q_SLOTS:
    void setAction(Qdl::UrlAction action);

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);

    void onSelectionChanged();
    
private:
    UrlActionModel *m_actionModel;
    
    QListView *m_view;

    QComboBox *m_actionSelector;

    QDialogButtonBox *m_buttonBox;

    QFormLayout *m_layout;
};

#endif // CLIPBOARDURLSDIALOG_H
