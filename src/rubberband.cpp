/***************************************************************************
  rubberband.cpp - Rubberband

 ---------------------
 begin                : 11.6.2016
 copyright            : (C) 2016 by Matthias Kuhn (OPENGIS.ch)
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rubberband.h"

#include "rubberbandmodel.h"
#include "sgrubberband.h"

Rubberband::Rubberband( QQuickItem* parent )
  : QQuickItem( parent )
  , mModel ( nullptr )
  , mDirty ( false )
  , mColor ( 192, 57, 43, 200 )
  , mWidth ( 1.5 )
{
  setFlags( QQuickItem::ItemHasContents );
  setAntialiasing( true );
}

RubberbandModel* Rubberband::model() const
{
  return mModel;
}

void Rubberband::setModel( RubberbandModel* model )
{
  if ( mModel == model )
    return;

  mModel = model;

  // TODO connect to changes

  connect( mModel, SIGNAL( vertexChanged( int ) ), this, SLOT( markDirty() ) );
  connect( mModel, SIGNAL( verticesRemoved( int,int ) ), this, SLOT( markDirty() ) );
  connect( mModel, SIGNAL( verticesInserted( int,int ) ), this, SLOT( markDirty() ) );

  markDirty();

  emit modelChanged();
}

MapCanvas* Rubberband::mapCanvas() const
{
  return mMapCanvas;
}

void Rubberband::setMapCanvas( MapCanvas* mapCanvas )
{
  if ( mMapCanvas == mapCanvas )
    return;

  mMapCanvas = mapCanvas;
  markDirty();

  emit mapCanvasChanged();
}

void Rubberband::markDirty()
{
  mDirty = true;
  update();
}

QSGNode* Rubberband::updatePaintNode( QSGNode* n, QQuickItem::UpdatePaintNodeData* )
{
  if ( mDirty && mModel )
  {
    delete n;
    n = new QSGNode;

    if ( !mModel->isEmpty() )
    {
      SGRubberband* rb = new SGRubberband( mModel->flatVertices(), mModel->geometryType(), mColor, mWidth );
      rb->setFlag( QSGNode::OwnedByParent );

      n->appendChildNode( rb );
    }
  }

  return n;
}

qreal Rubberband::width() const
{
  return mWidth;
}

void Rubberband::setWidth( qreal width )
{
  mWidth = width;
}

QColor Rubberband::color() const
{
  return mColor;
}

void Rubberband::setColor( const QColor& color )
{
  mColor = color;
}
