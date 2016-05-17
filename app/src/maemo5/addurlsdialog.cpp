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

#include "addurlsdialog.h"
#include "categoryselectionmodel.h"
#include "serviceselectionmodel.h"
#include "settings.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

AddUrlsDialog::AddUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(new ServiceSelectionModel(this)),
    m_categoryModel(new CategorySelectionModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_edit(new QTextEdit(m_container)),
    m_checkBox(new QCheckBox(tr("Check urls"), m_container)),
    m_serviceSelector(new ValueSelector(tr("Service"), m_container)),
    m_categorySelector(new ValueSelector(tr("Category"), m_container)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Add URLs"));
    setMinimumHeight(360);

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);
    
    m_edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    m_edit->setFocus(Qt::OtherFocusReason);

    m_checkBox->setChecked(Settings::checkUrls());
    
    m_serviceSelector->setModel(m_serviceModel);
    m_serviceSelector->setValue(Settings::defaultServicePlugin());
    
    m_categorySelector->setModel(m_categoryModel);
    m_categorySelector->setValue(Settings::defaultCategory());

    m_vbox->addWidget(m_edit);
    m_vbox->addWidget(m_checkBox);
    m_vbox->addWidget(m_serviceSelector);
    m_vbox->addWidget(m_categorySelector);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);

    connect(m_edit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void AddUrlsDialog::accept() {
    Settings::setCheckUrls(m_checkBox->isChecked());
    Settings::setDefaultCategory(m_categorySelector->currentValue().toString());
    Settings::setDefaultServicePlugin(m_serviceSelector->currentValue().toString()); 
    QDialog::accept();
}

QString AddUrlsDialog::text() const {
    return m_edit->toPlainText();
}

void AddUrlsDialog::setText(const QString &text) {
    m_edit->setText(text);
}

QStringList AddUrlsDialog::urls() const {
    return text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
}

void AddUrlsDialog::setUrls(const QStringList &u) {
    setText(u.join("\n") + "\n");
}

void AddUrlsDialog::addUrl(const QString &url) {
    m_edit->insertPlainText(url + "\n");
}

void AddUrlsDialog::addUrls(const QStringList &urls) {
    foreach (const QString &url, urls) {
        addUrl(url);
    }
}

void AddUrlsDialog::importUrls(const QString &fileName) {
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            const QUrl url = QUrl::fromUserInput(file.readLine());

            if (url.isValid()) {
                m_edit->insertPlainText(url.toString() + "\n");
            }
        }

        file.close();
    }
}

void AddUrlsDialog::onTextChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text().isEmpty());
}
