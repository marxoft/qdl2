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
#include <QListView>
#include <QVBoxLayout>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new SearchSelectionModel(this)),
    m_view(new QListView(this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Search"));
    setMinimumHeight(360);
    
    m_model->setTextAlignment(Qt::AlignCenter);
    
    m_view->setModel(m_model);
    m_view->setCurrentIndex(m_model->index(qMax(0, m_model->match(0, "value", Settings::defaultSearchPlugin()))));
    
    m_layout->addWidget(m_view);
    
    connect(m_view, SIGNAL(activated(QModelIndex)), this, SLOT(accept()));
}

QString SearchDialog::pluginName() const {
    return m_view->currentIndex().data(SelectionModel::NameRole).toString();
}

QString SearchDialog::pluginId() const {
    return m_view->currentIndex().data(SelectionModel::ValueRole).toString();
}

void SearchDialog::setPluginId(const QString &id) {
    m_view->setCurrentIndex(m_model->index(qMax(0, m_model->match(0, "value", id))));
}

void SearchDialog::accept() {
    Settings::setDefaultSearchPlugin(pluginId());
    QDialog::accept();
}
