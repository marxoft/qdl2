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

#include "multivalueselector.h"
#include "qmaemo5multilistpickselector.h"
#include "selectionmodel.h"

MultiValueSelector::MultiValueSelector(const QString &text, QWidget *parent) :
    QMaemo5ValueButton(text, parent),
    m_model(0),
    m_selector(new QMaemo5MultiListPickSelector(this))
{
    setPickSelector(m_selector);
    setValueText(tr("None chosen"));
    connect(m_selector, SIGNAL(selected(QString)), this, SLOT(onSelected()));
}

QList<int> MultiValueSelector::currentIndexes() const {
    return m_selector->currentIndexes();
}

void MultiValueSelector::setCurrentIndexes(const QList<int> &i) {
    m_selector->setCurrentIndexes(i);
}

SelectionModel* MultiValueSelector::model() const {
    return m_model;
}

void MultiValueSelector::setModel(SelectionModel *model) {
    m_model = model;
    m_selector->setModel(model);
    m_selector->setModelColumn(0);
    
    if (model) {
        model->setTextAlignment(Qt::AlignCenter);
    }
}

void MultiValueSelector::setValues(const QVariantList &v) {
    if (!m_model) {
        return;
    }

    QList<int> indexes;

    foreach (const QVariant &value, v) {
        const int i = m_model->match(0, "value", value);

        if (i != -1) {
            indexes << i;
        }
    }

    setCurrentIndexes(indexes);
}

QVariantList MultiValueSelector::currentValues() const {
    if (!m_model) {
        return QVariantList();
    }

    QVariantList values;

    foreach (const int &i, currentIndexes()) {
        values << m_model->data(i, "name");
    }

    return values;
}

void MultiValueSelector::onSelected() {
    if (currentIndexes().isEmpty()) {
        setValueText(tr("None chosen"));
    }

    emit valuesChanged(currentValues());
}
