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

#include "qmaemo5multilistpickselector.h"
#include <QAbstractButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QListView>
#include <QPainter>
#include <QStyledItemDelegate>

/*!
 * \internal
 * \class QMaemo5ListPickSelectorPrivate
 * \brief Private implementation for QMaemo5MultiListPickSelector
 */
class QMaemo5MultiListPickSelectorPrivate
{

public:
    /*!
     * \internal
     * Constructor
     */
    QMaemo5MultiListPickSelectorPrivate(QMaemo5MultiListPickSelector *parent) :
        q_ptr(parent),
        model(0),
        view(0),
        modelColumn(0)
    {
    }

    /*!
     * \internal
     * Destructor. Deletes the item view, but not the model.
     */
    ~QMaemo5MultiListPickSelectorPrivate() {
        if (view) {
            delete view;
        }
    }

    /*!
     * \internal
     * Emits the selected() signal of the pick selector.
     */
    void emitSelected() {
        Q_Q(QMaemo5MultiListPickSelector);
        emit q->selected(q->currentValueText());
    }

    /*!
     * \internal
     * Sets the model pointer to 0 and clears the current indexes.
     */
    void _q_modelDestroyed() {
        model = 0;
        currentIndexes.clear();
    }

    /*!
     * \internal
     * Sets the item view pointer to 0.
     */
    void _q_viewDestroyed() {
        view = 0;
    }

    QMaemo5MultiListPickSelector *q_ptr;
    QAbstractItemModel *model;
    QAbstractItemView *view;

    int modelColumn;

    QList<int> currentIndexes;

    Q_DECLARE_PUBLIC(QMaemo5MultiListPickSelector)
};

/*!
 * \internal
 * \class QMaemo5MultiListPickSelectorDialog
 * \brief The dialog used to display the item view of the pick selector.
 */
class QMaemo5MultiListPickSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \internal
     * Constructor.
     */
    explicit QMaemo5MultiListPickSelectorDialog(QMaemo5MultiListPickSelectorPrivate *selector,
            QWidget *parent) :
        QDialog(parent),
        m_selector(selector),
        m_itemView(0),
        m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
        m_layout(new QGridLayout(this))
    {
        updateLayout();
        connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(updateLayout()));
    }

    /*!
     * \internal
     * Destructor. If an item view was set on the pick selector, its parent is set to 0.
     */
    ~QMaemo5MultiListPickSelectorDialog() {
        if (m_selector->view) {
            m_selector->view->setParent(0);
        }
    }

    /*!
     * \internal
     * Returns the item view.
     */
    QAbstractItemView* itemView() const {
        return m_itemView;
    }

    /*!
     * \internal
     * Sets the item view.
     */
    void setItemView(QAbstractItemView *view) {
        if (view == m_itemView) {
            return;
        }

        if (m_itemView) {
            m_layout->removeWidget(m_itemView);
            m_itemView->setParent(0);
        }

        m_itemView = view;

        if (m_itemView) {
            m_layout->addWidget(m_itemView, 0, 0);
        }
    }

public Q_SLOTS:
    /*!
     * \internal
     * \reimp
     *
     * Sets the current indexes and emits the selected() signal of the pick selector.
     */
    void accept() {
        if (!m_itemView) {
            QDialog::accept();
            return;
        }
        
        m_selector->currentIndexes.clear();

        foreach (const QModelIndex &index, m_itemView->selectionModel()->selectedRows(m_selector->modelColumn)) {
            m_selector->currentIndexes.append(index.row());
        }

        qSort(m_selector->currentIndexes.begin(), m_selector->currentIndexes.end());
        m_selector->emitSelected();
        QDialog::accept();
    }

private Q_SLOTS:
    /*!
     * \internal
     * Updates the layout according to the screen geometry.
     */
    void updateLayout() {
        const QRect geometry = QApplication::desktop()->screenGeometry(this);

        m_layout->removeWidget(m_buttonBox);

        if (geometry.width() > geometry.height()) {
            // Landscape layout
            m_buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_layout->addWidget(m_buttonBox, 0, 1, Qt::AlignBottom);
        }
        else {
            // Portrait layout
            m_buttonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            m_layout->addWidget(m_buttonBox, 1, 0);
        }
    }

private:
    QMaemo5MultiListPickSelectorPrivate *m_selector;
    QAbstractItemView *m_itemView;
    QDialogButtonBox *m_buttonBox;
    QGridLayout *m_layout;
};

/*!
 * \internal
 * \class QMaemo5MultiListPickSelectorDelegate
 * \brief The delegate used for the item view of the pick selector.
 */
class QMaemo5MultiListPickSelectorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /*!
     * \internal
     * Constructor.
     */
    explicit QMaemo5MultiListPickSelectorDelegate(QObject *parent) :
        QStyledItemDelegate(parent)
    {
    }

    /*!
     * \internal
     * Destructor.
     */
    ~QMaemo5MultiListPickSelectorDelegate() {}

    /*!
     * \internal
     * \reimp
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const {
        QRect textRect = option.rect;

        if (option.state & QStyle::State_Selected) {
            textRect.setLeft(textRect.left() + 62);
            textRect.setRight(textRect.right() - 62);
            const int x = option.rect.right() - 56;
            const int y = option.rect.top() + 11;
            painter->drawPixmap(option.rect, QPixmap("/etc/hildon/theme/images/TouchListBackgroundPressed.png"));
            painter->drawPixmap(x, y, QPixmap("/usr/share/icons/hicolor/48x48/hildon/widgets_tickmark_list.png"));
        }
        else {
            textRect.setLeft(textRect.left() + 8);
            textRect.setRight(textRect.right() - 8);
            painter->drawPixmap(option.rect, QPixmap("/etc/hildon/theme/images/TouchListBackgroundNormal.png"));
        }

        const QString text = painter->fontMetrics().elidedText(index.data().toString(), Qt::ElideRight,
                textRect.width());
        painter->drawText(textRect, Qt::AlignCenter, text);
    }

    /*!
     * \internal
     * \reimp
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const {
        return QSize(option.rect.width(), 70);
    }
};

QMaemo5MultiListPickSelector::QMaemo5MultiListPickSelector(QObject *parent) :
    QMaemo5AbstractPickSelector(parent),
    d_ptr(new QMaemo5MultiListPickSelectorPrivate(this))
{
}

QMaemo5MultiListPickSelector::~QMaemo5MultiListPickSelector() {}

QWidget* QMaemo5MultiListPickSelector::widget(QWidget *parent) {
    Q_D(QMaemo5MultiListPickSelector);
    QMaemo5MultiListPickSelectorDialog *dialog = new QMaemo5MultiListPickSelectorDialog(d, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    if (const QAbstractButton *button = qobject_cast<QAbstractButton*>(parent)) {
        dialog->setWindowTitle(button->text());
    }

    QAbstractItemView *view = d->view;

    if (!view) {
        QListView *lv = new QListView(dialog);
        lv->setModel(d->model);
        lv->setModelColumn(d->modelColumn);
        lv->setItemDelegate(new QMaemo5MultiListPickSelectorDelegate(lv));
        lv->setSelectionMode(QAbstractItemView::MultiSelection);
        lv->setMinimumHeight(350);
        view = lv;
    }
    else {
        view->setModel(d->model);
    }

    dialog->setItemView(view);

    if ((d->model) && (!d->currentIndexes.isEmpty())) {
        for (int i = 0; i < d->currentIndexes.size(); i++) {
            view->selectionModel()->select(d->model->index(d->currentIndexes.at(i), d->modelColumn),
                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }

        view->scrollTo(d->model->index(d->currentIndexes.first(), d->modelColumn),
                QAbstractItemView::PositionAtCenter);
    }

    return dialog;
}

QAbstractItemModel* QMaemo5MultiListPickSelector::model() const {
    Q_D(const QMaemo5MultiListPickSelector);
    return d->model;
}

void QMaemo5MultiListPickSelector::setModel(QAbstractItemModel *m) {
    Q_D(QMaemo5MultiListPickSelector);

    if (m == d->model) {
        return;
    }

    if (d->model) {
        disconnect(d->model, SIGNAL(destroyed()), this, SLOT(_q_modelDestroyed()));
    }

    d->model = m;

    if (d->model) {
        connect(d->model, SIGNAL(destroyed()), this, SLOT(_q_modelDestroyed()));
    }

    d->currentIndexes.clear();
}

QAbstractItemView* QMaemo5MultiListPickSelector::view() const {
    Q_D(const QMaemo5MultiListPickSelector);
    return d->view;
}

void QMaemo5MultiListPickSelector::setView(QAbstractItemView *v) {
    Q_D(QMaemo5MultiListPickSelector);

    if (v == d->view) {
        return;
    }

    if (d->view) {
        disconnect(d->view, SIGNAL(destroyed()), this, SLOT(_q_viewDestroyed()));
    }

    d->view = v;

    if (d->view) {
        connect(d->view, SIGNAL(destroyed()), this, SLOT(_q_viewDestroyed()));
        d->view->setParent(0);
    }
}

int QMaemo5MultiListPickSelector::modelColumn() const {
    Q_D(const QMaemo5MultiListPickSelector);
    return d->modelColumn;
}

void QMaemo5MultiListPickSelector::setModelColumn(int column) {
    Q_D(QMaemo5MultiListPickSelector);
    d->modelColumn = column;
    emit selected(currentValueText());
}

QString QMaemo5MultiListPickSelector::currentValueText() const {
    Q_D(const QMaemo5MultiListPickSelector);

    if (!d->model) {
        return QString();
    }

    QString text;

    for (int i = 0; i < d->currentIndexes.size(); i++) {
        text.append(d->model->index(d->currentIndexes.at(i), d->modelColumn).data().toString());
        text.append(",");
    }

    text.chop(1);
    return text;
}

QList<int> QMaemo5MultiListPickSelector::currentIndexes() const {
    Q_D(const QMaemo5MultiListPickSelector);
    return d->currentIndexes;
}

void QMaemo5MultiListPickSelector::setCurrentIndexes(const QList<int> &indexes) {
    Q_D(QMaemo5MultiListPickSelector);

    if (!d->model) {
        return;
    }

    d->currentIndexes.clear();
    const int count = d->model->rowCount();

    for (int i = 0; i < indexes.size(); i++) {
        if ((indexes.at(i) >= 0) && (indexes.at(i) < count)) {
            d->currentIndexes.append(indexes.at(i));
        }
    }

    qSort(d->currentIndexes.begin(), d->currentIndexes.end());
    emit selected(currentValueText());
}

#include "moc_qmaemo5multilistpickselector.cpp"
#include "qmaemo5multilistpickselector.moc"

