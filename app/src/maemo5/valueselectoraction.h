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

#ifndef VALUESELECTORACTION_H
#define VALUESELECTORACTION_H

#include <QWidgetAction>

class SelectionModel;

class ValueSelectorAction : public QWidgetAction
{
    Q_OBJECT
    
    Q_PROPERTY(QString valueText READ valueText NOTIFY valueChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(QVariant currentValue READ currentValue WRITE setValue NOTIFY valueChanged)

public:
    explicit ValueSelectorAction(QWidget *parent = 0);
    explicit ValueSelectorAction(const QString &text, QWidget *parent = 0);

    SelectionModel* model() const;
    void setModel(SelectionModel *model);
    
    QString valueText() const;
    
    int currentIndex() const;

    QVariant currentValue() const;

    QWidget* createWidget(QWidget *parent);

public Q_SLOTS:
    void setCurrentIndex(int i);
    
    void setValue(const QVariant &v);
    
protected Q_SLOTS:
    void onValueChanged(const QVariant &v);

Q_SIGNALS:
    void valueChanged(const QVariant &v);

protected:
    SelectionModel *m_model;
    QString m_text;
    int m_index;
    QVariant m_value;
};

#endif // VALUESELECTORACTION_H
