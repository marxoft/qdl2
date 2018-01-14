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

#include "valueselectoraction.h"
#include "valueselector.h"
#include "selectionmodel.h"

ValueSelectorAction::ValueSelectorAction(QWidget *parent) :
    QWidgetAction(parent),
    m_index(0)
{
}

ValueSelectorAction::ValueSelectorAction(const QString &text, QWidget *parent) :
    QWidgetAction(parent),
    m_index(0)
{
    setText(text);
}

int ValueSelectorAction::currentIndex() const {
    return m_index;
}

void ValueSelectorAction::setCurrentIndex(int i) {
    if (i != currentIndex()) {
        m_index = i;
        
        if (m_model) {
            setValue(m_model->data(m_index, "value"));
        }
    }
}

SelectionModel* ValueSelectorAction::model() const {
    return m_model;
}

void ValueSelectorAction::setModel(SelectionModel *model) {
    m_model = model;
}

QString ValueSelectorAction::valueText() const {
    return m_text;
}

QVariant ValueSelectorAction::currentValue() const {
    return m_value;
}

void ValueSelectorAction::setValue(const QVariant &v) {
    if (v != currentValue()) {
        m_value = v;
        emit valueChanged(v);
        
        if (m_model) {
            m_index = qMax(0, m_model->match(0, "value", v));
        }
    }
}

QWidget* ValueSelectorAction::createWidget(QWidget *parent) {
    ValueSelector *selector = new ValueSelector(text(), parent);

    if (m_model) {
        selector->setModel(m_model);
        selector->setValue(currentValue());
    }
    
    m_text = selector->valueText();
    connect(selector, SIGNAL(valueChanged(QVariant)), this, SLOT(onValueChanged(QVariant)));

    return selector;
}

void ValueSelectorAction::onValueChanged(const QVariant &v) {    
    if (ValueSelector *selector = qobject_cast<ValueSelector*>(sender())) {
        m_text = selector->valueText();
    }
    
    setValue(v);
}
