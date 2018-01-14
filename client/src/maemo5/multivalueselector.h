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

#ifndef MULTIVALUESELECTOR_H
#define MULTIVALUESELECTOR_H

#include <QMaemo5ValueButton>
#include <QVariantList>

class QMaemo5MultiListPickSelector;
class SelectionModel;

class MultiValueSelector : public QMaemo5ValueButton
{
    Q_OBJECT
    
    Q_PROPERTY(QList<int> currentIndexes READ currentIndexes WRITE setCurrentIndexes)
    Q_PROPERTY(QVariantList currentValues READ currentValues WRITE setValues NOTIFY valuesChanged)

public:
    explicit MultiValueSelector(const QString &text, QWidget *parent = 0);

    QList<int> currentIndexes() const;
    
    SelectionModel* model() const;
    void setModel(SelectionModel *model);
    
    QVariantList currentValues() const;

public Q_SLOTS:
    void setCurrentIndexes(const QList<int> &i);
    
    void setValues(const QVariantList &v);

private Q_SLOTS:
    void onSelected();

Q_SIGNALS:
    void valuesChanged(const QVariantList &v);

private:
    SelectionModel *m_model;
    QMaemo5MultiListPickSelector *m_selector;
};

#endif // MULTIVALUESELECTOR_H
