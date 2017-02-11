/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

class SearchSelectionModel;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;

class SearchDialog : public QDialog
{
    Q_OBJECT
    
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(QString pluginId READ pluginId WRITE setPluginId)

public:
    explicit SearchDialog(QWidget *parent = 0);
        
    QString pluginName() const;
    QString pluginId() const;

public Q_SLOTS:    
    void setPluginId(const QString &id);
    
    virtual void accept();

private:
    SearchSelectionModel *m_selectionModel;
        
    QComboBox *m_pluginSelector;
    
    QDialogButtonBox *m_buttonBox;
    
    QFormLayout *m_layout;
};

#endif // SEARCHDIALOG_H
