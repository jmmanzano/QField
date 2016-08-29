/***************************************************************************
                            qgismobileapp.cpp
                              -------------------
              begin                : Wed Apr 04 10:48:28 CET 2012
              copyright            : (C) 2012 by Marco Bernasocchi
              email                : marco@bernawebdesign.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>

#include <unistd.h>

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickItem>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog> // Until native looking QML dialogs are implemented (Qt5.4?)
#include <QtWidgets/QMenu> // Until native looking QML dialogs are implemented (Qt5.4?)
#include <QtWidgets/QMenuBar>
#include <QStandardItemModel>

#include <qgsproject.h>
#include <qgsmaplayerregistry.h>
#include <qgsmaptoolidentify.h>
#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <qgssnappingutils.h>

#include "qgismobileapp.h"
#include "qgsquickmapcanvasmap.h"
#include "appinterface.h"
#include "featurelistmodelselection.h"
#include "featurelistmodelhighlight.h"
#include "qgseditorwidgetregistry.h"
#include "maptransform.h"
#include "featurelistextentcontroller.h"
#include "coordinatetransform.h"
#include "modelhelper.h"
#include "rubberband.h"
#include "rubberbandmodel.h"
#include "qgsofflineediting.h"
#include "messagelogmodel.h"
#include "attributeformmodel.h"
#include "geometry.h"
#include "featuremodel.h"
#include "layertreemapcanvasbridge.h"

QgisMobileapp::QgisMobileapp( QgsApplication *app, QWindow *parent )
  : QQuickView( parent )
  , mIface( new AppInterface( this ) )
  , mFirstRenderingFlag( true )
{
#if 0
  QSurfaceFormat format;
  format.setSamples( 8 );
  setFormat( format );
  create();
#endif

  mLayerTree = new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), this );

  initDeclarative();

  QgsEditorWidgetRegistry::initEditors();

  setSource( QUrl( "qrc:/qml/qgismobileapp.qml" ) );

  connect( engine(), SIGNAL( quit() ), app, SLOT( quit() ) );

  setMinimumHeight( 480 );
  setMinimumWidth( 800 );

  QgsQuickMapCanvasMap* mMapCanvas = rootObject()->findChild<QgsQuickMapCanvasMap*>();

  Q_ASSERT_X( mMapCanvas, "QML Init", "QgsQuickMapCanvasMap not found. It is likely that we failed to load the QML files. Check debug output for related messages." );

  connect( this, SIGNAL( closing( QQuickCloseEvent* ) ), QgsApplication::instance(), SLOT( quit() ) );

  connect( QgsProject::instance(), SIGNAL( readProject( QDomDocument ) ), this, SLOT( onReadProject( QDomDocument ) ) );

  mLayerTreeCanvasBridge = new LayerTreeMapCanvasBridge( QgsProject::instance()->layerTreeRoot(), mMapCanvas, this );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument& ) ), mLayerTreeCanvasBridge, SLOT( writeProject( QDomDocument& ) ) );
  connect( QgsProject::instance(), SIGNAL( readProject( QDomDocument ) ), mLayerTreeCanvasBridge, SLOT( readProject( QDomDocument ) ) );
  connect( this, SIGNAL( loadProjectStarted( QString ) ), mIface, SIGNAL( loadProjectStarted( QString ) ) );
  connect( this, SIGNAL( loadProjectEnded() ), mIface, SIGNAL( loadProjectEnded() ) );
  connect( this, SIGNAL( afterRendering() ), SLOT( onAfterFirstRendering() ), Qt::QueuedConnection );

  mOfflineEditing = new QgsOfflineEditing();

  mSettings.setValue( "/Map/searchRadiusMM", 5 );

  show();
}

void QgisMobileapp::initDeclarative()
{
  // Register QGIS QML types
  qmlRegisterUncreatableType<QgsProject>( "org.qgis", 1, 0, "Project", "" );
  qmlRegisterType<QgsQuickMapCanvasMap>( "org.qgis", 1, 0, "MapCanvasMap" );
  qmlRegisterType<QgsSnappingUtils>( "org.qgis", 1, 0, "SnappingUtils" );
  qmlRegisterType<QgsMapLayerProxyModel>( "org.qgis", 1, 0, "MapLayerModel" );
  qmlRegisterType<QgsVectorLayer>( "org.qgis", 1, 0, "VectorLayer" );
  qRegisterMetaType<QgsWkbTypes::GeometryType>( "QgsWkbTypes::GeometryType" );
  qRegisterMetaType<QgsFeatureId>( "QgsFeatureId" );
  qRegisterMetaType<QgsAttributes>( "QgsAttributes" );

  // Register QField QML types
  qmlRegisterUncreatableType<AppInterface>( "org.qgis", 1, 0, "QgisInterface", "QgisInterface is only provided by the environment and cannot be created ad-hoc" );
  qmlRegisterUncreatableType<Settings>( "org.qgis", 1, 0, "Settings", "" );
  qmlRegisterUncreatableType<PlatformUtilities>( "org.qgis", 1, 0, "PlatformUtilities", "" );
  qmlRegisterType<FeatureListModel>( "org.qgis", 1, 0, "FeatureListModel" );
  qmlRegisterType<FeatureListModelSelection>( "org.qgis", 1, 0, "FeatureListModelSelection" );
  qmlRegisterType<FeatureListModelHighlight>( "org.qgis", 1, 0, "FeatureListModelHighlight" );
  qmlRegisterType<MapTransform>( "org.qgis", 1, 0, "MapTransform" );
  qmlRegisterType<FeatureListExtentController>( "org.qgis", 1, 0, "FeaturelistExtentController" );
  qmlRegisterType<CoordinateTransform>( "org.qgis", 1, 0, "CoordinateTransform" );
  qmlRegisterType<CRS>( "org.qgis", 1, 0, "CRS" );
  qmlRegisterType<Geometry>( "org.qgis", 1, 0, "Geometry" );
  qmlRegisterType<ModelHelper>( "org.qgis", 1, 0, "ModelHelper" );
  qmlRegisterType<Rubberband>( "org.qgis", 1, 0, "Rubberband" );
  qmlRegisterType<RubberbandModel>( "org.qgis", 1, 0, "RubberbandModel" );
  qmlRegisterType<PictureSource>( "org.qgis", 1, 0, "PictureSource" );
  qmlRegisterType<MessageLogModel>( "org.qgis", 1, 0, "MessageLogModel" );
  qmlRegisterType<AttributeFormModel>( "org.qfield", 1, 0, "AttributeFormModel" );
  qmlRegisterType<FeatureModel>( "org.qfield", 1, 0, "FeatureModel" );

  // Calculate device pixels
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int dpi = dpiX < dpiY ? dpiX : dpiY; // In case of asymetrical DPI. Improbable
  float dp = dpi * 0.00768443;

  // Register some globally available variables
  rootContext()->setContextProperty( "dp", dp );
  rootContext()->setContextProperty( "project", QgsProject::instance() );
  rootContext()->setContextProperty( "iface", mIface );
  rootContext()->setContextProperty( "featureListModel", &mFeatureListModel );
  rootContext()->setContextProperty( "settings", &mSettings );
  rootContext()->setContextProperty( "version", QString( "" VERSTR ) );
  rootContext()->setContextProperty( "layerTree", mLayerTree );
  rootContext()->setContextProperty( "project", QgsProject::instance() );
  rootContext()->setContextProperty( "platformUtilities", &mPlatformUtils );
}

void QgisMobileapp::loadProjectQuirks()
{
  // force update of canvas, without automatic changes to extent and OTF projections
  bool autoEnableCrsTransform = mLayerTreeCanvasBridge->autoEnableCrsTransform();
  bool autoSetupOnFirstLayer = mLayerTreeCanvasBridge->autoSetupOnFirstLayer();
  mLayerTreeCanvasBridge->setAutoEnableCrsTransform( false );
  mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( false );

  mLayerTreeCanvasBridge->setCanvasLayers();

  if ( autoEnableCrsTransform )
    mLayerTreeCanvasBridge->setAutoEnableCrsTransform( true );

  if ( autoSetupOnFirstLayer )
    mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( true );
}

void QgisMobileapp::identifyFeatures( const QPointF& point )
{
  // TODO: FIX IDENTIFY
#if 0
  QgsMapToolIdentify identify( mMapCanvas );
  QList<QgsMapToolIdentify::IdentifyResult> results = identify.identify( point.x(), point.y(), QgsMapToolIdentify::TopDownAll, QgsMapToolIdentify::VectorLayer );

  mFeatureListModel.setFeatures( results );
  mIface->openFeatureForm();
#endif
}

void QgisMobileapp::onReadProject( const QDomDocument& doc )
{
  Q_UNUSED( doc );
  QMap<QgsVectorLayer*, QgsFeatureRequest> requests;
  QSettings().setValue( "/qgis/project/lastProjectFile", QgsProject::instance()->fileName() );
  Q_FOREACH( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer );
    if ( vl )
    {
      const QVariant itinerary = vl->customProperty( "qgisMobile/itinerary" );
      if ( itinerary.isValid() )
      {
        requests.insert( vl, QgsFeatureRequest().setFilterExpression( itinerary.toString() ) );
      }
    }
  }
  if ( requests.count() )
  {
    qDebug() << QString( "Loading itinerary for %1 layers." ).arg( requests.count() );
    mFeatureListModel.setFeatures( requests );
    mIface->openFeatureForm();
  }
}

void QgisMobileapp::onAfterFirstRendering()
{
  // This should get triggered exactly once, so we disconnect it right away
  disconnect( this, SIGNAL( afterRendering() ), this, SLOT( onAfterFirstRendering() ) );

  if ( mFirstRenderingFlag )
  {
    if ( qApp->arguments().count() > 1 )
    {
      loadProjectFile( qApp->arguments().last() );
    }
    else
    {
      QTimer::singleShot( 0, this, SLOT( loadLastProject() ) );
    }
    mFirstRenderingFlag = false;
  }
}

void QgisMobileapp::loadLastProject()
{
  QVariant lastProjectFile = QSettings().value( "/qgis/project/lastProjectFile" );
  if ( lastProjectFile.isValid() )
    loadProjectFile( lastProjectFile.toString() );
}

void QgisMobileapp::loadProjectFile( const QString& path )
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  emit loadProjectStarted( path );
  QgsProject::instance()->read( path );
  loadProjectQuirks();

  emit loadProjectEnded();
}

QgisMobileapp::~QgisMobileapp()
{
  delete mOfflineEditing;
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  // Reintroduce when created on the heap
  // delete QgsEditorWidgetRegistry::instance();
  delete QgsProject::instance();
}
