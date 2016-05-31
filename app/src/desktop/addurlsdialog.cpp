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
#include "settings.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QFile>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>

AddUrlsDialog::AddUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_categoryModel(new CategorySelectionModel(this)),
    m_headerModel(new SelectionModel(this)),
    m_tabBar(new QTabBar(this)),
    m_stack(new QStackedWidget(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this)),
    m_urlsTab(new QWidget(m_stack)),
    m_urlsEdit(new QTextEdit(m_urlsTab)),
    m_categorySelector(new QComboBox(m_urlsTab)),
    m_pluginCheckBox(new QCheckBox(tr("Use &plugins"), m_urlsTab)),
    m_urlsLayout(new QFormLayout(m_urlsTab)),
    m_methodTab(0),
    m_methodEdit(0),
    m_postEdit(0),
    m_methodLayout(0),
    m_headersTab(0),
    m_headerView(0),
    m_headerNameEdit(0),
    m_headerValueEdit(0),
    m_headerButton(0),
    m_headerLayout(0),
    m_method("GET")
    
{
    setWindowTitle(tr("Add URLs"));
    setAcceptDrops(true);
    
    m_tabBar->addTab(tr("URLs"));
    m_tabBar->addTab(tr("Method"));
    m_tabBar->addTab(tr("Headers"));
    m_tabBar->setTabEnabled(1, !Settings::usePlugins());
    m_tabBar->setTabEnabled(2, !Settings::usePlugins());

    m_urlsEdit->setFocus(Qt::OtherFocusReason);
    
    m_categorySelector->setModel(m_categoryModel);
    m_categorySelector->setCurrentIndex(m_categorySelector->findData(Settings::defaultCategory()));
    
    m_pluginCheckBox->setChecked(Settings::usePlugins());
    
    m_urlsLayout->addRow(m_urlsEdit);
    m_urlsLayout->addRow(tr("&Category:"), m_categorySelector);
    m_urlsLayout->addRow(m_pluginCheckBox);
    
    m_stack->addWidget(m_urlsTab);
    
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    
    m_layout->addWidget(m_tabBar);
    m_layout->addWidget(m_stack);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTab(int)));
    connect(m_urlsEdit, SIGNAL(textChanged()), this, SLOT(onUrlsChanged()));
    connect(m_pluginCheckBox, SIGNAL(toggled(bool)), this, SLOT(onUsePluginsChanged(bool)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    showUrlsTab();
}

void AddUrlsDialog::accept() {
    Settings::setDefaultCategory(m_categorySelector->itemData(m_categorySelector->currentIndex()).toString());
    Settings::setUsePlugins(m_pluginCheckBox->isChecked());
    QDialog::accept();
}

QString AddUrlsDialog::postData() const {
    return m_postData;
}

void AddUrlsDialog::setPostData(const QString &data) {
    m_postData = data;
    
    if (m_postEdit) {
        m_postEdit->setText(data);
    }
}

QVariantMap AddUrlsDialog::requestHeaders() const {
    QVariantMap headers;
    
    for (int i = 0; i < m_headerModel->rowCount(); i++) {
        const QModelIndex idx = m_headerModel->index(i, 0);
        headers[idx.data(SelectionModel::NameRole).toString()] = idx.data(SelectionModel::ValueRole);
    }
    
    return headers;
}

void AddUrlsDialog::setRequestHeaders(const QVariantMap &headers) {
    m_headerModel->clear();

    if (!headers.isEmpty()) {    
        QMapIterator<QString, QVariant> iterator(headers);
        
        while (iterator.hasNext()) {
            iterator.next();
            m_headerModel->append(iterator.key(), iterator.value());
        }
    }
}

void AddUrlsDialog::addRequestHeader(const QString &name, const QVariant &value) {
    m_headerModel->append(name, value);
}

void AddUrlsDialog::addRequestHeader() {
    addRequestHeader(m_headerNameEdit->text(), m_headerValueEdit->text());
    m_headerNameEdit->clear();
    m_headerValueEdit->clear();
}

QString AddUrlsDialog::requestMethod() const {
    return m_method;
}

void AddUrlsDialog::setRequestMethod(const QString &method) {
    m_method = method.isEmpty() ? QString("GET") : method;
    
    if (m_methodEdit) {
        m_methodEdit->setText(m_method);
    }
}

QString AddUrlsDialog::text() const {
    return m_urlsEdit->toPlainText();
}

void AddUrlsDialog::setText(const QString &text) {
    m_urlsEdit->setText(text);
}

QStringList AddUrlsDialog::urls() const {
    return text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
}

void AddUrlsDialog::setUrls(const QStringList &u) {
    setText(u.join("\n") + "\n");
}

void AddUrlsDialog::addUrl(const QString &url) {
    m_urlsEdit->insertPlainText(url + "\n");
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
                addUrl(url.toString());
            }
        }

        file.close();
    }
}

bool AddUrlsDialog::usePlugins() const {
    return m_pluginCheckBox->isChecked();
}

void AddUrlsDialog::setUsePlugin(bool enabled) {
    m_pluginCheckBox->setChecked(enabled);
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

void AddUrlsDialog::setCurrentTab(int index) {
    switch (index) {
    case 0:
        showUrlsTab();
        break;
    case 1:
        showMethodTab();
        break;
    case 2:
        showHeadersTab();
        break;
    default:
        break;
    }
}

void AddUrlsDialog::showUrlsTab() {
    m_stack->setCurrentWidget(m_urlsTab);
}

void AddUrlsDialog::showMethodTab() {
    if (!m_methodTab) {
        m_methodTab = new QWidget(m_stack);
        m_methodEdit = new QLineEdit(requestMethod(), m_methodTab);
        m_methodEdit->setFocus(Qt::OtherFocusReason);
        m_postEdit = new QLineEdit(postData(), m_methodTab);
        m_methodLayout = new QFormLayout(m_methodTab);
        m_methodLayout->addRow(tr("&Method:"), m_methodEdit);
        m_methodLayout->addRow(tr("Post &data:"), m_postEdit);
        m_stack->addWidget(m_methodTab);

        connect(m_methodEdit, SIGNAL(textChanged(QString)), this, SLOT(onRequestMethodChanged(QString)));
        connect(m_postEdit, SIGNAL(textChanged(QString)), this, SLOT(onPostDataChanged(QString)));
    }

    m_stack->setCurrentWidget(m_methodTab);
}

void AddUrlsDialog::showHeadersTab() {
    if (!m_headersTab) {
        m_headersTab = new QWidget(m_stack);
        m_headerView = new QTreeView(m_headersTab);
        m_headerView->setModel(m_headerModel);
        m_headerView->setAlternatingRowColors(true);
        m_headerView->setSelectionBehavior(QTreeView::SelectRows);
        m_headerView->setContextMenuPolicy(Qt::CustomContextMenu);
        m_headerView->setEditTriggers(QTreeView::NoEditTriggers);
        m_headerView->setItemsExpandable(false);
        m_headerView->setUniformRowHeights(true);
        m_headerView->setAllColumnsShowFocus(true);
        m_headerView->setRootIsDecorated(false);
        m_headerView->header()->setStretchLastSection(true);
        m_headerNameEdit = new QLineEdit(m_headersTab);
        m_headerNameEdit->setFocus(Qt::OtherFocusReason);
        m_headerValueEdit = new QLineEdit(m_headersTab);
        m_headerButton = new QPushButton(QIcon::fromTheme("list-add"), tr("Add"), m_headersTab);
        m_headerButton->setEnabled(false);
        m_headerLayout = new QFormLayout(m_headersTab);
        m_headerLayout->addRow(m_headerView);
        m_headerLayout->addRow(new QLabel(tr("Add header:"), m_headersTab));
        m_headerLayout->addRow(tr("&Name:"), m_headerNameEdit);
        m_headerLayout->addRow(tr("&Value:"), m_headerValueEdit);
        m_headerLayout->addWidget(m_headerButton);
        m_stack->addWidget(m_headersTab);
        
        connect(m_headerView, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showRequestHeaderContextMenu(QPoint)));
        connect(m_headerNameEdit, SIGNAL(textChanged(QString)), this, SLOT(onRequestHeaderNameChanged(QString)));
        connect(m_headerValueEdit, SIGNAL(textChanged(QString)), this, SLOT(onRequestHeaderValueChanged(QString)));
        connect(m_headerButton, SIGNAL(clicked()), this, SLOT(addRequestHeader()));
    }
    
    m_stack->setCurrentWidget(m_headersTab);
}

void AddUrlsDialog::showRequestHeaderContextMenu(const QPoint &pos) {
    if (!m_headerView->currentIndex().isValid()) {
        return;
    }
    
    QMenu menu(this);
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("Remove"));
    
    if (menu.exec(m_headerView->mapToGlobal(pos))) {
        m_headerModel->remove(m_headerView->currentIndex().row());
    }
}

void AddUrlsDialog::onPostDataChanged(const QString &data) {
    m_postData = data;
}

void AddUrlsDialog::onRequestHeaderNameChanged(const QString &name) {
    m_headerButton->setEnabled((!name.isEmpty()) && (!m_headerValueEdit->text().isEmpty()));
}

void AddUrlsDialog::onRequestHeaderValueChanged(const QString &value) {
    m_headerButton->setEnabled((!value.isEmpty()) && (!m_headerNameEdit->text().isEmpty()));
}

void AddUrlsDialog::onRequestMethodChanged(const QString &method) {
    m_method = method;
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(((!method.isEmpty()) || (usePlugins()))
                                                          && (!text().isEmpty()));
}

void AddUrlsDialog::onUrlsChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled((!text().isEmpty()) && ((usePlugins())
                                                          || (!requestMethod().isEmpty())));
}

void AddUrlsDialog::onUsePluginsChanged(bool enabled) {
    m_tabBar->setTabEnabled(1, !enabled);
    m_tabBar->setTabEnabled(2, !enabled);
}
