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

#include "graphicsview.h"
#include <QDeclarativeItem>

class GraphicsViewPrivate
{

public:
    GraphicsViewPrivate(GraphicsView *parent) :
        q_ptr(parent),
        root(0)
    {
    }
    
    GraphicsView *q_ptr;
    QDeclarativeItem *root;
    
    Q_DECLARE_PUBLIC(GraphicsView)
};

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    d_ptr(new GraphicsViewPrivate(this))
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(0);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    viewport()->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::StrongFocus);
    
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setStickyFocus(true);
}

GraphicsView::~GraphicsView() {}

void GraphicsView::addItem(QDeclarativeItem *item) {
    Q_D(GraphicsView);
    
    if (!d->root) {
        d->root = item;
    }
    
    scene()->addItem(item);
}

void GraphicsView::resizeEvent(QResizeEvent *e) {
    Q_D(GraphicsView);
    
    if (d->root) {
        if (!qFuzzyCompare(width(), d->root->width())) {
            d->root->setWidth(width());
        }

        if (!qFuzzyCompare(height(), d->root->height())) {
            d->root->setHeight(height());
        }

        setSceneRect(QRectF(0, 0, d->root->width(), d->root->height()));
    }
    
    updateGeometry();
    QGraphicsView::resizeEvent(e);
}

#include "moc_graphicsview.cpp"
