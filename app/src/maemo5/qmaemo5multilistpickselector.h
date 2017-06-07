/*!
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

#ifndef QMAEMO5MULTILISTPICKSELECTOR_H
#define QMAEMO5MULTILISTPICKSELECTOR_H

#include <QMaemo5AbstractPickSelector>

class QAbstractItemModel;
class QAbstractItemView;
class QMaemo5MultiListPickSelectorPrivate;

/*!
 * \class QMaemo5MultiListPickSelector
 * \brief The QMaemo5MultiListPickSelector widget displays a list of items.
 *
 * The QMaemo5MultiListPickSelector is a selector similar to QMaemo5ListPickSelector that displays the
 * contents of a QAbstractItemModel and allows the user to select one or more items from it.
 *
 * This object should be used together with the QMaemo5ValueButton.
 *
 * \sa QAbstractItemModel, QMaemo5ListPickSelector, QMaemo5ValueButton
 */
class QMaemo5MultiListPickSelector : public QMaemo5AbstractPickSelector
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel)
    Q_PROPERTY(int modelColumn READ modelColumn WRITE setModelColumn)
    Q_PROPERTY(QList<int> currentIndexes READ currentIndexes WRITE setCurrentIndexes)

public:
    /*!
     * Constructs a new multi list pick selector with the given \a parent object.
     */
    explicit QMaemo5MultiListPickSelector(QObject *parent = 0);

    /*!
     * Destroys the pick selector and the view (if set).
     * It will not destroy the model.
     */
    ~QMaemo5MultiListPickSelector();

    /*!
     * \reimp
     */
    QWidget* widget(QWidget *parent);

    /*!
     * \property QMaemo5MultiListPickSelector::model
     * \brief The model used in the list picker.
     *
     * Either list or table models can be used. Table models may require the \l modelColumn
     * property to be set to a non-default value, depending on their contents.
     *
     * The use of tree models with a pick selector is untested. In addition, changing the model
     * while the list picker is displayed may lead to undefined behavior.
     *
     * \note The list picker will not take ownership of the model.
     */
    QAbstractItemModel* model() const;
    void setModel(QAbstractItemModel *model);

    /*!
     * \brief Returns the item view used in the list picker.
     */
    QAbstractItemView* view() const;

    /*!
     * \brief Sets the item view used in the list picker to \a itemView. The list picker
     * takes ownership of the item view.
     *
     * \note You must set the selectionMode of the item view to either
     * QAbstractItemView::ExtendedSelection or QAbstractItemView::MultiSelection if you want
     * to enable selection of more than one item. Tne default item view uses
     * QAbstractItemView::MultiSelection.
     * 
     * If you want to use the convenience views (like QListWidget, QTableWidget
     * or QTreeWidget), make sure you call setModel() on the list picker with the
     * convenience widget's model before calling this function.
     */
    void setView(QAbstractItemView *itemView);

    /*!
     * \property QMaemo5MultiListPickSelector::modelColumn
     * \brief The column in the model that is used for the picker results.
     *
     * This property determines the column in a table model used for determining
     * the picker result.
     *
     * By default, this property has a value of \c 0.
     */
    int modelColumn() const;
    void setModelColumn(int column);

    /*!
     * \reimp
     */
    QString currentValueText() const;

    /*!
     * \property QMaemo5MultiListPickSelector::currentIndexes
     * \brief The indexes of the selected items in the list picker.
     *
     * By default, this property is an empty list.
     */
    QList<int> currentIndexes() const;

public Q_SLOTS:
    void setCurrentIndexes(const QList<int> &indexes);

private:
    QScopedPointer<QMaemo5MultiListPickSelectorPrivate> d_ptr;

    Q_DISABLE_COPY(QMaemo5MultiListPickSelector)
    Q_DECLARE_PRIVATE(QMaemo5MultiListPickSelector)

    Q_PRIVATE_SLOT(d_func(), void _q_modelDestroyed())
    Q_PRIVATE_SLOT(d_func(), void _q_viewDestroyed())
};

#endif // QMAEMO5MULTILISTPICKSELECTOR_H

