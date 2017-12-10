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

#include "texteditdialog.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    m_label(new QLabel(tr("Text"), this)),
    m_edit(new QTextEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("Enter text"));

    m_layout->addWidget(m_label, 0, 0);
    m_layout->addWidget(m_edit, 1, 0);
    m_layout->addWidget(m_buttonBox, 1, 1);

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

TextEditDialog::TextEditDialog(const QString &text, QWidget *parent) :
    QDialog(parent),
    m_label(new QLabel(tr("Text"), this)),
    m_edit(new QTextEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("Enter text"));
    setText(text);

    m_layout->addWidget(m_label, 0, 0);
    m_layout->addWidget(m_edit, 1, 0);
    m_layout->addWidget(m_buttonBox, 1, 1);

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QString TextEditDialog::labelText() const {
    return m_label->text();
}

void TextEditDialog::setLabelText(const QString &text) {
    m_label->setText(text);
}

QString TextEditDialog::text() const {
    return m_edit->toPlainText();
}

void TextEditDialog::setText(const QString &text) {
    if (Qt::mightBeRichText(text)) {
        m_edit->setHtml(text);
    }
    else {
        m_edit->setPlainText(text);
    }
}

QString TextEditDialog::getText(QWidget *parent, const QString &title, const QString &labelText,
        const QString &text) {
    TextEditDialog dialog(text, parent);
    dialog.setWindowTitle(title);
    dialog.setLabelText(labelText);
    
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.text();
    }

    return QString();
}
