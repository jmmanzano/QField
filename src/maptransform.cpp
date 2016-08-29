/***************************************************************************

               ----------------------------------------------------
              date                 : 20.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "maptransform.h"
#include "qgsquickmapcanvasmap.h"

#include <QDebug>

MapTransform::MapTransform()
{
}

MapTransform::~MapTransform()
{
}

void MapTransform::applyTo( QMatrix4x4* matrix ) const
{
  *matrix *= mMatrix;
  matrix->optimize();
}

QgsQuickMapCanvasMap* MapTransform::mapCanvas() const
{
  return mMapCanvas;
}

void MapTransform::setMapCanvas( QgsQuickMapCanvasMap* mapSettings )
{
  if ( mapSettings != mMapCanvas )
  {
    mMapCanvas = mapSettings;
    connect( mMapCanvas, SIGNAL( extentChanged() ), SLOT( updateMatrix() ) );
    emit mapCanvasChanged();
  }
}

void MapTransform::updateMatrix()
{
  QMatrix4x4 matrix;
  float scaleFactor = 1 / mMapCanvas->mapSettings().mapUnitsPerPixel();

  matrix.scale( scaleFactor, -scaleFactor );
  matrix.translate( -mMapCanvas->mapSettings().visibleExtent().xMinimum(), -mMapCanvas->mapSettings().visibleExtent().yMaximum() );

  mMatrix = matrix;
  update();
}
