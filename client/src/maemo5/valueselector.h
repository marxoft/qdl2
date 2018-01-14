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

#ifndef VALUESELECTOR_H
#define VALUESELECTOR_H

#include <QMaemo5ValueButton>
#include <QVariant>

class QMaemo5ListPickSelector;
class SelectionModel;

class ValueSelector : public QMaemo5ValueButton
{
    Q_OBJECT
    
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(QVariant currentValue READ currentValue WRITE setValue NOTIFY valueChanged)

public:
    explicit ValueSelector(const QString &text, QWidget *parent = 0);

    int currentIndex() const;
    
    SelectionModel* model() const;
    void setModel(SelectionModel *model);
    
    QVariant currentValue() const;

public Q_SLOTS:
    void setCurrentIndex(int i);
    
    void setValue(const QVariant &v);

private Q_SLOTS:
    void onSelected();

Q_SIGNALS:
    void valueChanged(const QVariant &v);

private:
    void showEvent(QShowEvent *e);

    SelectionModel *m_model;
    QMaemo5ListPickSelector *m_selector;
};

#endif // VALUESELECTOR_H
