/***************************************************************************

               ----------------------------------------------------
              date                 : 27.12.2014
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

#include "featurelistextentcontroller.h"
#include "qgsquickmapcanvasmap.h"

#include <qgsvectorlayer.h>
#include <qgsgeometry.h>

FeatureListExtentController::FeatureListExtentController( QObject* parent )
  : QObject( parent )
  , mModel( 0 )
  , mSelection( 0 )
  , mMapCanvas( 0 )
  , mAutoZoom( false )
{
  connect( this, SIGNAL( autoZoomChanged() ), SLOT( zoomToSelected() ) );
  connect( this, SIGNAL( modelChanged() ), SLOT( onModelChanged() ) );
  connect( this, SIGNAL( selectionChanged() ), SLOT( onModelChanged() ) );
}

FeatureListExtentController::~FeatureListExtentController()
{
}

void FeatureListExtentController::zoomToSelected() const
{
  if ( mModel && mSelection && mMapCanvas )
  {
    QgsFeature feat = mSelection->selectedFeature();
    QgsVectorLayer* layer = mSelection->selectedLayer();

    QgsCoordinateTransform transf( layer->crs(), mMapCanvas->mapSettings().destinationCrs() );
    QgsGeometry geom( feat.geometry() );
    geom.transform( transf );

    QgsRectangle featureExtent = geom.boundingBox();
    QgsRectangle bufferedExtent = featureExtent.buffer( qMax( featureExtent.width(), featureExtent.height() ) );

    mMapCanvas->setExtent( bufferedExtent );
  }
}

void FeatureListExtentController::onModelChanged()
{
  if ( mModel && mSelection )
  {
    connect( mSelection, SIGNAL( selectionChanged() ), SLOT( onCurrentSelectionChanged() ) );
  }
}

void FeatureListExtentController::onCurrentSelectionChanged()
{
  if ( mAutoZoom )
  {
    zoomToSelected();
  }
}
