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

#include "textinputdialog.h"
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVariant>
#include <QVBoxLayout>

TextInputDialog::TextInputDialog(QWidget *parent) :
    QDialog(parent),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);
    
    m_vbox->setContentsMargins(0, 0, 0, 0);
    
    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QString TextInputDialog::field(const QString &name, const QString &defaultValue) const {
    return m_fields.value(name, defaultValue);
}

void TextInputDialog::addField(const QString &name, const QString &value, QLineEdit::EchoMode echoMode) {
    QLineEdit *edit = new QLineEdit(value, m_container);
    edit->setEchoMode(echoMode);
    edit->setProperty("name", name);

    if (m_fields.isEmpty()) {
        edit->setFocus(Qt::OtherFocusReason);
    }
    
    connect(edit, SIGNAL(textChanged(QString)), this, SLOT(onFieldChanged(QString)));
    
    m_vbox->addWidget(new QLabel(name, m_container));
    m_vbox->addWidget(edit);
    m_fields[name] = value;
}

void TextInputDialog::onFieldChanged(const QString &value) {
    if (const QObject *obj = sender()) {
        m_fields[obj->property("name").toString()] = value;
    }
}
