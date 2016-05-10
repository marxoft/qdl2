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
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QFile>
#include <QFormLayout>
#include <QLabel>
#include <QMimeData>
#include <QPushButton>
#include <QTextEdit>

AddUrlsDialog::AddUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(new ServiceSelectionModel(this)),
    m_categoryModel(new CategorySelectionModel(this)),
    m_edit(new QTextEdit(this)),
    m_checkBox(new QCheckBox(tr("Check &urls"), this)),
    m_serviceSelector(new QComboBox(this)),
    m_categorySelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Add URLs"));
    setAcceptDrops(true);

    m_checkBox->setChecked(Settings::checkUrls());
    
    m_serviceSelector->setModel(m_serviceModel);
    m_serviceSelector->setCurrentIndex(m_serviceSelector->findData(Settings::defaultServicePlugin()));
    
    m_categorySelector->setModel(m_categoryModel);
    m_categorySelector->setCurrentIndex(m_categorySelector->findData(Settings::defaultCategory()));

    m_layout->addRow(m_edit);
    m_layout->addRow(m_checkBox);
    m_layout->addRow(tr("&Service:"), m_serviceSelector);
    m_layout->addRow(tr("&Category:"), m_categorySelector);
    m_layout->addRow(m_buttonBox);

    connect(m_edit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void AddUrlsDialog::accept() {
    Settings::setCheckUrls(m_checkBox->isChecked());
    Settings::setDefaultCategory(m_categorySelector->itemData(m_categorySelector->currentIndex()).toString());
    Settings::setDefaultServicePlugin(m_serviceSelector->itemData(m_serviceSelector->currentIndex()).toString()); 
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

void AddUrlsDialog::dragEnterEvent(QDragEnterEvent *event) {
    if ((event->mimeData()->hasUrls()) && (event->mimeData()->urls().first().path().toLower().endsWith(".txt"))) {
        event->acceptProposedAction();
    }
}

void AddUrlsDialog::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        const QString fileName = event->mimeData()->urls().first().path();

        if ((QFile::exists(fileName)) && (fileName.toLower().endsWith(".txt"))) {
            importUrls(fileName);
        }
    }
}

void AddUrlsDialog::onTextChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text().isEmpty());
}
