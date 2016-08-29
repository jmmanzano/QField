/***************************************************************************

               ----------------------------------------------------
              date                 : 27.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias.kuhn (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FEATURELISTEXTENTCONTROLLER_H
#define FEATURELISTEXTENTCONTROLLER_H

#include <QObject>

#include "featurelistmodel.h"
#include "featurelistmodelselection.h"

class QgsQuickMapCanvasMap;

class FeatureListExtentController : public QObject
{
    Q_OBJECT

    Q_PROPERTY( FeatureListModel* model MEMBER mModel NOTIFY modelChanged )
    Q_PROPERTY( FeatureListModelSelection* selection MEMBER mSelection NOTIFY selectionChanged )
    Q_PROPERTY( bool autoZoom MEMBER mAutoZoom NOTIFY autoZoomChanged )
    Q_PROPERTY( QgsQuickMapCanvasMap* mapCanvas MEMBER mMapCanvas NOTIFY mapCanvasChanged )

  public:
    FeatureListExtentController( QObject* parent = 0 );
    ~FeatureListExtentController();

  public slots:
    void zoomToSelected() const;

  signals:
    void autoZoomChanged();
    void selectionChanged();
    void modelChanged();
    void mapCanvasChanged();

  private slots:
    void onModelChanged();
    void onCurrentSelectionChanged();

  private:
    FeatureListModel* mModel;
    FeatureListModelSelection* mSelection;
    QgsQuickMapCanvasMap* mMapCanvas;
    bool mAutoZoom;
};

#endif // FEATURELISTEXTENTCONTROLLER_H
