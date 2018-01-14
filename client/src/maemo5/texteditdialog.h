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

#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QTextEdit;

class TextEditDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit TextEditDialog(QWidget *parent = 0);
    explicit TextEditDialog(const QString &text, QWidget *parent = 0);

    QString labelText() const;
    QString text() const;

    static QString getText(QWidget *parent, const QString &title, const QString &labelText,
            const QString &text = QString());

public Q_SLOTS:
    void setLabelText(const QString &text);
    void setText(const QString &text);

private:
    QLabel *m_label;

    QTextEdit *m_edit;

    QDialogButtonBox *m_buttonBox;

    QGridLayout *m_layout;
};

#endif // TEXTEDITDIALOG_H
