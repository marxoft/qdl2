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

#include "searchdialog.h"
#include "searchselectionmodel.h"
#include "settings.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPushButton>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    m_selectionModel(new SearchSelectionModel(this)),
    m_pluginSelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Search"));
    
    m_pluginSelector->setModel(m_selectionModel);
    m_pluginSelector->setCurrentIndex(qMax(0, m_pluginSelector->findData(Settings::defaultSearchPlugin())));
    
    m_layout->addRow(tr("&Service:"), m_pluginSelector);
    m_layout->addRow(m_buttonBox);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QString SearchDialog::pluginName() const {
    return m_pluginSelector->currentText();
}

QString SearchDialog::pluginId() const {
    return m_pluginSelector->itemData(m_pluginSelector->currentIndex()).toString();
}

void SearchDialog::setPluginId(const QString &id) {
    m_pluginSelector->setCurrentIndex(qMax(0, m_pluginSelector->findData(id)));
}

void SearchDialog::accept() {
    Settings::setDefaultSearchPlugin(pluginId());
    QDialog::accept();
}
