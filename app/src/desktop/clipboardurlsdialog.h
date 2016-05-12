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

#include <QDialog>

class QDialogButtonBox;
class QListView;
class QVBoxLayout;

class ClipboardUrlsDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QStringList urls READ urls)

public:
    explicit ClipboardUrlsDialog(QWidget *parent = 0);

    QStringList urls() const;

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);

    void onSelectionChanged();
    
private:
    QListView *m_view;

    QDialogButtonBox *m_buttonBox;

    QVBoxLayout *m_layout;
};

#endif // CLIPBOARDURLSDIALOG_H
