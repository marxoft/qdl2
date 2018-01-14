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

#ifndef DECAPTCHAPLUGINSDIALOG_H
#define DECAPTCHAPLUGINSDIALOG_H

#include <QDialog>

class DecaptchaPluginConfigModel;
class QHBoxLayout;
class QListView;
class QModelIndex;

class DecaptchaPluginsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DecaptchaPluginsDialog(QWidget *parent = 0);

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);
    void showPluginDialog(const QModelIndex &index);

private:
    DecaptchaPluginConfigModel *m_model;

    QListView *m_view;

    QHBoxLayout *m_layout;
};

#endif // DECAPTCHAPLUGINSDIALOG_H
