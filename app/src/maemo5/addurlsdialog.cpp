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
#include "textinputdialog.h"
#include "transferitemprioritymodel.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

AddUrlsDialog::AddUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_headerModel(new SelectionModel(this)),
    m_tabBar(new QTabBar(this)),
    m_stack(new QStackedWidget(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QGridLayout(this)),
    m_urlsEdit(new QTextEdit(m_stack)),
    m_settingsTab(0),
    m_categorySelector(0),
    m_prioritySelector(0),
    m_commandEdit(0),
    m_subfolderCheckBox(0),
    m_commandCheckBox(0),
    m_pluginCheckBox(0),
    m_autoCheckBox(0),
    m_methodTab(0),
    m_methodEdit(0),
    m_postEdit(0),
    m_headersTab(0),
    m_headerView(0),
    m_headerButton(0),
    m_category(Settings::defaultCategory()),
    m_method("GET"),
    m_createSubfolder(Settings::createSubfolders()),
    m_customCommandOverrideEnabled(false),
    m_usePlugins(Settings::usePlugins()),
    m_startAutomatically(Settings::startTransfersAutomatically()),
    m_priority(TransferItem::NormalPriority)
{
    setWindowTitle(tr("Add URLs"));
    setMinimumHeight(360);
    
    m_tabBar->setExpanding(false);
    m_tabBar->setStyleSheet("QTabBar::tab { height: 40px; }");
    m_tabBar->addTab(tr("URLs"));
    m_tabBar->addTab(tr("Settings"));
    m_tabBar->addTab(tr("Method"));
    m_tabBar->addTab(tr("Headers"));
    m_tabBar->setTabEnabled(2, !Settings::usePlugins());
    m_tabBar->setTabEnabled(3, !Settings::usePlugins());

    m_urlsEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    m_urlsEdit->setFocus(Qt::OtherFocusReason);
    
    m_stack->addWidget(m_urlsEdit);

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    
    m_layout->addWidget(m_tabBar, 0, 0);
    m_layout->addWidget(m_stack, 1, 0);
    m_layout->addWidget(m_buttonBox, 1, 1);
    m_layout->setRowStretch(1, 1);

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTab(int)));
    connect(m_urlsEdit, SIGNAL(textChanged()), this, SLOT(onUrlsChanged()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    showUrlsTab();
}

void AddUrlsDialog::accept() {
    Settings::setDefaultCategory(category());
    Settings::setCreateSubfolders(createSubfolder());
    Settings::setUsePlugins(usePlugins());
    Settings::setStartTransfersAutomatically(startAutomatically());
    QDialog::accept();
}

QString AddUrlsDialog::category() const {
    return m_category;
}

void AddUrlsDialog::setCategory(const QString &category) {
    m_category = category;

    if (m_categorySelector) {
        m_categorySelector->setValue(category);
    }
}

bool AddUrlsDialog::createSubfolder() const {
    return m_createSubfolder;
}

void AddUrlsDialog::setCreateSubfolder(bool enabled) {
    m_createSubfolder = enabled;

    if (m_subfolderCheckBox) {
        m_subfolderCheckBox->setChecked(enabled);
    }
}

QString AddUrlsDialog::customCommand() const {
    return m_customCommand;
}

void AddUrlsDialog::setCustomCommand(const QString &command) {
    m_customCommand = command;

    if (m_commandEdit) {
        m_commandEdit->setText(command);
    }
}

bool AddUrlsDialog::customCommandOverrideEnabled() const {
    return m_customCommandOverrideEnabled;
}

void AddUrlsDialog::setCustomCommandOverrideEnabled(bool enabled) {
    m_customCommandOverrideEnabled = enabled;

    if (m_commandCheckBox) {
        m_commandCheckBox->setChecked(enabled);
    }
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

TransferItem::Priority AddUrlsDialog::priority() const {
    return m_priority;
}

void AddUrlsDialog::setPriority(TransferItem::Priority priority) {
    m_priority = priority;

    if (m_prioritySelector) {
        m_prioritySelector->setValue(priority);
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
    TextInputDialog dialog(this);
    dialog.setWindowTitle(tr("Add header"));
    dialog.addField(tr("Name"));
    dialog.addField(tr("Value"));
    
    if (dialog.exec() == QDialog::Accepted) {
        const QString name = dialog.field(tr("Name"));
        const QString value = dialog.field(tr("Value"));
        
        if ((!name.isEmpty()) && (!value.isEmpty())) {
            addRequestHeader(name, value);
        }
    }
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

bool AddUrlsDialog::startAutomatically() const {
    return m_startAutomatically;
}

void AddUrlsDialog::setStartAutomatically(bool enabled) {
    m_startAutomatically = enabled;

    if (m_autoCheckBox) {
        m_autoCheckBox->setChecked(enabled);
    }
}

QString AddUrlsDialog::text() const {
    return m_urlsEdit->toPlainText();
}

void AddUrlsDialog::setText(const QString &text) {
    m_urlsEdit->setText(text);
}

QStringList AddUrlsDialog::urls() const {
    return text().split("\n", QString::SkipEmptyParts);
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
    return m_usePlugins;
}

void AddUrlsDialog::setUsePlugins(bool enabled) {
    m_usePlugins = enabled;

    if (m_pluginCheckBox) {
        m_pluginCheckBox->setChecked(enabled);
    }
}

void AddUrlsDialog::setCurrentTab(int index) {
    switch (index) {
    case 0:
        showUrlsTab();
        break;
    case 1:
        showSettingsTab();
        break;
    case 2:
        showMethodTab();
        break;
    case 3:
        showHeadersTab();
        break;
    default:
        break;
    }
}

void AddUrlsDialog::showUrlsTab() {
    m_stack->setCurrentWidget(m_urlsEdit);
}

void AddUrlsDialog::showMethodTab() {
    if (!m_methodTab) {
        m_methodTab = new QWidget(m_stack);
        m_methodEdit = new QLineEdit(requestMethod(), m_methodTab);
        m_methodEdit->setFocus(Qt::OtherFocusReason);
        m_postEdit = new QLineEdit(postData(), m_methodTab);
        QVBoxLayout *layout = new QVBoxLayout(m_methodTab);
        layout->addWidget(new QLabel(tr("Method"), m_methodTab));
        layout->addWidget(m_methodEdit);
        layout->addWidget(new QLabel(tr("Post data"), m_methodTab));
        layout->addWidget(m_postEdit);
        layout->setContentsMargins(0, 0, 0, 0);
        m_stack->addWidget(m_methodTab);
        
        connect(m_methodEdit, SIGNAL(textChanged(QString)), this, SLOT(onRequestMethodChanged(QString)));
        connect(m_postEdit, SIGNAL(textChanged(QString)), this, SLOT(onPostDataChanged(QString)));
    }

    m_stack->setCurrentWidget(m_methodTab);
}

void AddUrlsDialog::showSettingsTab() {
    if (!m_settingsTab) {
        m_settingsTab = new QScrollArea(m_stack);
        QWidget *widget = new QWidget(m_settingsTab);
        m_categorySelector = new ValueSelector(tr("Category"), widget);
        m_categorySelector->setModel(new CategorySelectionModel(m_categorySelector));
        m_categorySelector->setValue(category());
        m_prioritySelector = new ValueSelector(tr("Priority"), widget);
        m_prioritySelector->setModel(new TransferItemPriorityModel(m_prioritySelector));
        m_prioritySelector->setValue(priority());
        m_commandEdit = new QLineEdit(widget);
        m_commandEdit->setText(customCommand());
        m_subfolderCheckBox = new QCheckBox(tr("Create subfolder"), widget);
        m_subfolderCheckBox->setChecked(createSubfolder());
        m_commandCheckBox = new QCheckBox(tr("Override global custom command"), widget);
        m_commandCheckBox->setChecked(customCommandOverrideEnabled());
        m_pluginCheckBox = new QCheckBox(tr("Use plugins"), widget);
        m_pluginCheckBox->setChecked(usePlugins());
        m_autoCheckBox = new QCheckBox(tr("Start automatically"), widget);
        m_autoCheckBox->setChecked(startAutomatically());
        QVBoxLayout *layout = new QVBoxLayout(widget);
        layout->addWidget(m_subfolderCheckBox);
        layout->addWidget(m_categorySelector);
        layout->addWidget(m_prioritySelector);
        layout->addWidget(new QLabel(tr("Custom command (%f for filename)"), widget));
        layout->addWidget(m_commandEdit);
        layout->addWidget(m_commandCheckBox);
        layout->addWidget(m_pluginCheckBox);
        layout->addWidget(m_autoCheckBox);
        layout->setContentsMargins(0, 0, 0, 0);
        m_settingsTab->setWidget(widget);
        m_settingsTab->setWidgetResizable(true);
        m_stack->addWidget(m_settingsTab);

        connect(m_categorySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(onCategoryChanged(QVariant)));
        connect(m_prioritySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(onPriorityChanged(QVariant)));
        connect(m_commandEdit, SIGNAL(textChanged(QString)), this, SLOT(onCustomCommandChanged(QString)));
        connect(m_subfolderCheckBox, SIGNAL(toggled(bool)), this, SLOT(onCreateSubfolderChanged(bool)));
        connect(m_commandCheckBox, SIGNAL(toggled(bool)), this, SLOT(onCustomCommandOverrideEnabledChanged(bool)));
        connect(m_pluginCheckBox, SIGNAL(toggled(bool)), this, SLOT(onUsePluginsChanged(bool)));
        connect(m_autoCheckBox, SIGNAL(toggled(bool)), this, SLOT(onStartAutomaticallyChanged(bool)));
    }

    m_stack->setCurrentWidget(m_settingsTab);
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
        m_headerButton = new QPushButton(QIcon::fromTheme("general_add"), tr("Add"), m_headersTab);
        QVBoxLayout *layout = new QVBoxLayout(m_headersTab);
        layout->addWidget(m_headerView);
        layout->addWidget(m_headerButton);
        layout->setContentsMargins(0, 0, 0, 0);
        m_stack->addWidget(m_headersTab);
        
        connect(m_headerView, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showRequestHeaderContextMenu(QPoint)));
        connect(m_headerButton, SIGNAL(clicked()), this, SLOT(addRequestHeader()));
    }

    m_stack->setCurrentWidget(m_headersTab);
}

void AddUrlsDialog::showRequestHeaderContextMenu(const QPoint &pos) {
    if (!m_headerView->currentIndex().isValid()) {
        return;
    }
    
    QMenu menu(this);
    menu.addAction(tr("Remove"));
    
    if (menu.exec(mapToGlobal(pos))) {
        m_headerModel->remove(m_headerView->currentIndex().row());
    }
}

void AddUrlsDialog::onCategoryChanged(const QVariant &category) {
    m_category = category.toString();
}

void AddUrlsDialog::onCreateSubfolderChanged(bool enabled) {
    m_createSubfolder = enabled;
}

void AddUrlsDialog::onCustomCommandChanged(const QString &command) {
    m_customCommand = command;
}

void AddUrlsDialog::onCustomCommandOverrideEnabledChanged(bool enabled) {
    m_customCommandOverrideEnabled = enabled;
}

void AddUrlsDialog::onPostDataChanged(const QString &data) {
    m_postData = data;
}

void AddUrlsDialog::onPriorityChanged(const QVariant &priority) {
    m_priority = TransferItem::Priority(priority.toInt());
}

void AddUrlsDialog::onRequestMethodChanged(const QString &method) {
    m_method = method;
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(((!method.isEmpty()) || (usePlugins()))
                                                          && (!text().isEmpty()));
}

void AddUrlsDialog::onStartAutomaticallyChanged(bool enabled) {
    m_startAutomatically = enabled;
}

void AddUrlsDialog::onUrlsChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled((!text().isEmpty()) && ((usePlugins())
                                                          || (!requestMethod().isEmpty())));
}

void AddUrlsDialog::onUsePluginsChanged(bool enabled) {
    m_tabBar->setTabEnabled(2, !enabled);
    m_tabBar->setTabEnabled(3, !enabled);
}
