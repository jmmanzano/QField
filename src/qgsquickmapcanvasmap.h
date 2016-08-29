/***************************************************************************
                            qgsquickmapcanvasmap.h
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias.kuhn (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSQUICKMAPCANVASMAP_H
#define QGSQUICKMAPCANVASMAP_H

#include <QtQuick/QQuickPaintedItem>
#include <QTimer>
#include <qgspoint.h>
#include <qgsmapsettings.h>

class QgsMapRendererParallelJob;
class QgsMapRendererCache;
class QgsLabelingResults;

class QgsQuickMapCanvasMap : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )

  public:
    QgsQuickMapCanvasMap( QQuickItem* parent = 0 );
    ~QgsQuickMapCanvasMap();

    // QQuickPaintedItem interface
    void paint( QPainter* painter );

    QgsPoint toMapCoordinates( QPoint canvasCoordinates );

    QgsMapSettings mapSettings() const;

    QgsUnitTypes::DistanceUnit mapUnits() const;
    void setMapUnits( const QgsUnitTypes::DistanceUnit& mapUnits );

    QList<QgsMapLayer*> layerSet() const;
    void setLayerSet( const QList<QgsMapLayer*>& layerSet );

    bool hasCrsTransformEnabled() const;
    void setCrsTransformEnabled( bool hasCrsTransformEnabled );

    QgsCoordinateReferenceSystem destinationCrs() const;
    void setDestinationCrs( const QgsCoordinateReferenceSystem& destinationCrs );

    QgsRectangle extent() const;
    void setExtent( const QgsRectangle& extent );

  signals:
    void mapSettingsChanged();

    void renderStarting();

    void mapCanvasRefreshed();

    void extentChanged();

    void destinationCrsChanged();

  protected:
    void geometryChanged( const QRectF& newGeometry, const QRectF& oldGeometry );

  public slots:
    void stopRendering();

    void zoomToFullExtent();

    void zoom( QPointF center, qreal scale );
    void pan( QPointF oldPos, QPointF newPos );
    void refresh();

  private slots:
    void refreshMap();
    void renderJobFinished();

  private:
    bool mPinching;
    bool mJobCancelled;
    bool mRefreshScheduled;
    QPoint mPinchStartPoint;
    QgsMapSettings mMapSettings;
    QgsMapRendererParallelJob* mJob;
    QgsMapRendererCache* mCache;
    QgsLabelingResults* mLabelingResults;
    QImage mImg;
};

#endif // QGSQUICKMAPCANVASMAP_H
