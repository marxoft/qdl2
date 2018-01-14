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

#ifndef TEXTINPUTDIALOG_H
#define TEXTINPUTDIALOG_H

#include <QDialog>
#include <QHash>
#include <QLineEdit>

class QDialogButtonBox;
class QHBoxLayout;
class QScrollArea;
class QVBoxLayout;

class TextInputDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TextInputDialog(QWidget *parent = 0);
    
    QString field(const QString &name, const QString &defaultValue = QString()) const;
    
public Q_SLOTS:
    void addField(const QString &name, const QString &value = QString(),
                  QLineEdit::EchoMode echoMode = QLineEdit::Normal);

private Q_SLOTS:
    void onFieldChanged(const QString &value);

private:
    QScrollArea *m_scrollArea;
    
    QWidget *m_container;
    
    QDialogButtonBox *m_buttonBox;
    
    QVBoxLayout *m_vbox;
    QHBoxLayout *m_layout;
    
    QHash<QString, QString> m_fields;
};

#endif // TEXTINPUTDIALOG_H
