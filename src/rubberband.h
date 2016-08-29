/***************************************************************************
  rubberband.h - Rubberband

 ---------------------
 begin                : 11.6.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RUBBERBAND_H
#define RUBBERBAND_H

#include <QQuickItem>

class RubberbandModel;
class MapCanvas;

class Rubberband : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY( RubberbandModel* model READ model WRITE setModel NOTIFY modelChanged )
    Q_PROPERTY( MapCanvas* mapCanvas READ mapCanvas WRITE setMapCanvas NOTIFY mapCanvasChanged )
    Q_PROPERTY( QColor color READ color WRITE setColor NOTIFY colorChanged )
    Q_PROPERTY( qreal width READ width WRITE setWidth NOTIFY widthChanged )

  public:
    Rubberband( QQuickItem* parent = nullptr );

    RubberbandModel* model() const;
    void setModel( RubberbandModel* model );

    MapCanvas* mapCanvas() const;
    void setMapCanvas( MapCanvas* mapCanvas );

    QColor color() const;
    void setColor( const QColor& color );

    qreal width() const;
    void setWidth( qreal width );

  signals:
    void modelChanged();
    void mapCanvasChanged();
    void colorChanged();
    void widthChanged();

  private slots:
    void markDirty();

  private:
    QSGNode* updatePaintNode( QSGNode* n, QQuickItem::UpdatePaintNodeData* );

    RubberbandModel* mModel;
    MapCanvas* mMapCanvas;
    bool mDirty;
    QColor mColor;
    qreal mWidth;
};

#endif // RUBBERBAND_H
