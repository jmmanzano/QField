/***************************************************************************
                            qgsquickmapcanvasmap.cpp
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickmapcanvasmap.h"

#include <qgsmaprendererparalleljob.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgsmessagelog.h>

QgsQuickMapCanvasMap::QgsQuickMapCanvasMap(  QQuickItem* parent )
  : QQuickPaintedItem( parent )
  , mPinching( false )
{
  setRenderTarget( QQuickPaintedItem::FramebufferObject );
}

QgsQuickMapCanvasMap::~QgsQuickMapCanvasMap()
{
}

void QgsQuickMapCanvasMap::paint( QPainter* painter )
{
  ////
}

QgsMapSettings QgsQuickMapCanvasMap::mapSettings() const
{
  return mMapSettings;
}

void QgsQuickMapCanvasMap::zoom( QPointF center, qreal scale )
{
  /*
  QgsPoint oldCenter( mMapCanvas->mapSettings().visibleExtent().center() );
  QgsPoint mousePos( mMapCanvas->getCoordinateTransform()->toMapPoint( center.x(), center.y() ) );
  QgsPoint newCenter( mousePos.x() + ( ( oldCenter.x() - mousePos.x() ) * scale ),
                      mousePos.y() + ( ( oldCenter.y() - mousePos.y() ) * scale ) );

  // same as zoomWithCenter (no coordinate transformations are needed)
  QgsRectangle extent = mMapCanvas->mapSettings().visibleExtent();
  extent.scale( scale, &newCenter );
  mMapCanvas->setExtent( extent );

  update();
  */
}

void QgsQuickMapCanvasMap::pan( QPointF oldPos, QPointF newPos )
{
  /*

  QgsPoint start = mMapCanvas->getCoordinateTransform()->toMapCoordinates( oldPos.toPoint() );
  QgsPoint end = mMapCanvas->getCoordinateTransform()->toMapCoordinates( newPos.toPoint() );

  double dx = qAbs( end.x() - start.x() );
  double dy = qAbs( end.y() - start.y() );

  // modify the extent
  QgsRectangle r = mMapCanvas->mapSettings().visibleExtent();

  if ( end.x() > start.x() )
  {
    r.setXMinimum( r.xMinimum() + dx );
    r.setXMaximum( r.xMaximum() + dx );
  }
  else
  {
    r.setXMinimum( r.xMinimum() - dx );
    r.setXMaximum( r.xMaximum() - dx );
  }

  if ( end.y() > start.y() )
  {
    r.setYMaximum( r.yMaximum() + dy );
    r.setYMinimum( r.yMinimum() + dy );

  }
  else
  {
    r.setYMaximum( r.yMaximum() - dy );
    r.setYMinimum( r.yMinimum() - dy );

  }

  mMapCanvas->setExtent( r );

  update();
  */
}

void QgsQuickMapCanvasMap::refreshMap()
{
  Q_ASSERT( mRefreshScheduled );

  stopRendering(); // if any...

  // from now on we can accept refresh requests again
  mRefreshScheduled = false;

  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope()
                    << QgsExpressionContextUtils::mapSettingsScope( mMapSettings );

  mMapSettings.setExpressionContext( expressionContext );

  // create the renderer job
  Q_ASSERT( !mJob );
  mJobCancelled = false;
  mJob = new QgsMapRendererParallelJob( mMapSettings );
  connect( mJob, SIGNAL( finished() ), SLOT( rendererJobFinished() ) );
  mJob->setCache( mCache );

  QStringList layersForGeometryCache;
  Q_FOREACH ( const QString& id, mMapSettings.layers() )
  {
    if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( id ) ) )
    {
      if ( vl->isEditable() )
        layersForGeometryCache << id;
    }
  }
  mJob->setRequestedGeometryCacheForLayers( layersForGeometryCache );

  mJob->start();

  emit renderStarting();
}

void QgsQuickMapCanvasMap::renderJobFinished()
{
  Q_FOREACH ( const QgsMapRendererJob::Error& error, mJob->errors() )
  {
    QgsMessageLog::logMessage( error.layerID + " :: " + error.message, tr( "Rendering" ) );
  }

  if ( !mJobCancelled )
  {
    // take labeling results before emitting renderComplete, so labeling map tools
    // connected to signal work with correct results
    delete mLabelingResults;
    mLabelingResults = mJob->takeLabelingResults();

    mImg = mJob->renderedImage();
  }

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = nullptr;

  emit mapCanvasRefreshed();
}

QgsRectangle QgsQuickMapCanvasMap::extent() const
{
  return mMapSettings.extent();
}

void QgsQuickMapCanvasMap::setExtent( const QgsRectangle& extent )
{
  if ( mMapSettings.extent() == extent )
    return;

  mMapSettings.setExtent( extent );
  emit extentChanged();
}

QgsCoordinateReferenceSystem QgsQuickMapCanvasMap::destinationCrs() const
{
  return mMapSettings.destinationCrs();
}

void QgsQuickMapCanvasMap::setDestinationCrs( const QgsCoordinateReferenceSystem& destinationCrs )
{
  if ( mMapSettings.destinationCrs() == destinationCrs )
    return;

  mMapSettings.setDestinationCrs( destinationCrs );
  emit destinationCrsChanged();
}

void QgsQuickMapCanvasMap::geometryChanged( const QRectF& newGeometry, const QRectF& oldGeometry )
{
// TDOD
}

bool QgsQuickMapCanvasMap::hasCrsTransformEnabled() const
{
  return mMapSettings.hasCrsTransformEnabled();
}

void QgsQuickMapCanvasMap::setCrsTransformEnabled( bool crsTransformEnabled )
{
  if ( mMapSettings.hasCrsTransformEnabled() == crsTransformEnabled )
    return;

  mMapSettings.setCrsTransformEnabled( crsTransformEnabled );
}

QList<QgsMapLayer*> QgsQuickMapCanvasMap::layerSet() const
{
  QList<QgsMapLayer*> mapLayers;

  Q_FOREACH( const QString& id, mMapSettings.layers() )
  {
    QgsMapLayer* lyr = QgsMapLayerRegistry::instance()->mapLayer( id );
    if ( lyr )
      mapLayers << lyr;
  }

  return mapLayers;
}

void QgsQuickMapCanvasMap::setLayerSet( const QList<QgsMapLayer*>& layerSet )
{
  QStringList ids;
  Q_FOREACH( QgsMapLayer* layer, layerSet )
    ids << layer->id();

  mMapSettings.setLayers( ids );
}

QgsUnitTypes::DistanceUnit QgsQuickMapCanvasMap::mapUnits() const
{
  return mMapSettings.mapUnits();
}

void QgsQuickMapCanvasMap::setMapUnits( const QgsUnitTypes::DistanceUnit& mapUnits )
{
  if ( mMapSettings.mapUnits() == mapUnits )
    return;

  mMapSettings.setMapUnits( mapUnits );
}

void QgsQuickMapCanvasMap::stopRendering()
{
  if ( mJob )
  {
    mJobCancelled = true;
    mJob->cancel();
    Q_ASSERT( !mJob ); // no need to delete here: already deleted in finished()
  }
}

void QgsQuickMapCanvasMap::zoomToFullExtent()
{
// TODO
}

void QgsQuickMapCanvasMap::refresh()
{
  if ( !mMapSettings.hasValidSettings() )
    return;

  if ( mRefreshScheduled )
    return;

  mRefreshScheduled = true;

  QTimer::singleShot( 1, this, SLOT( refreshMap() ) );
}
