#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPainter>
#include <featex.h>
#include <os/fstools.h>
//#include "os/persistence.h"
#include <QPen>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QtCore/qmath.h>
#include "projectmanager.h"
#include <QSharedPointer>
#include <QInputDialog>
#include <QColorDialog>


class ProjectManager;



MouseMarker::MouseMarker( ){};
void MouseMarker::mousePressed( QPoint p, Qt::MouseButton b )
{
	button = b;
	p1 = p;
	p2 = p;
	state=Pressed;
	emit check();
};
void MouseMarker::mouseReleased( QPoint p )
{
	p2=p;
	state=Released;
	emit check();
};
void MouseMarker::mouseMoved( QPoint p )
{
	p2=p;
	state=Moved;
	emit check();
};


void MainWindow::savePersistence()
{
	QMap<QString, QString> data;
	data["_currentProjectPath"] = _currentProjectPath;
	data["_lastLoadedDir"] = _lastLoadedDir;
	FSTools::mapToFile( data, "c:/tmp/cellapp.cfg", "=");
}


void MainWindow::loadPersistence()
{
	QMap<QString, QString> data = FSTools::mapFromFile( "c:/tmp/cellapp.cfg", QRegExp("=") );
	_currentProjectPath = data["_currentProjectPath"];
	_lastLoadedDir = data["_lastLoadedDir"];
	if ( !_currentProjectPath.isEmpty() )
	{
		openProject( _currentProjectPath );
	}

	if ( !_lastLoadedDir.isEmpty() )
	{
		loadDir( _lastLoadedDir );
	}
}


QString MainWindow::currentClass( )
{
	QString c;
	if ( _classesSelection->selectedRows().size()==1 )
	{
		QModelIndex ix = _classesSelection->selectedRows().first();
		ix = _classesSelection->model()->index( ix.row(), CCName );
		c = ix.data().toString();
	}
	
	return c;
}

QColor MainWindow::currentClassCol( )
{
	QColor c;
	if ( _classesSelection->selectedRows().size()==1 )
	{
		QModelIndex ix = _classesSelection->selectedRows().first();
		ix = _classesSelection->model()->index( ix.row(), CCColor );
		c = ix.data(Qt::DecorationRole).toString();
	}
	
	return c;
}

QPolygon _lastPolygon;

void MainWindow::checkCellMarker( )
{
	QString c = currentClass( );
	if ( c.isEmpty() )
		return;

	MouseMarker *m = qobject_cast<MouseMarker*>( sender() );
	if(m)
	{
		if ( m->button == Qt::RightButton )
		{
			if (m->state == MouseMarker::Pressed )
			{
				_project->removeAt( c, this->_currentImage, m->p1 );

				 foreach ( QGraphicsItem* r, _recognizedMarkers)
				 {
					QGraphicsRectItem* rr =static_cast<QGraphicsRectItem*>(r);
					if ( rr->boundingRect().contains(m->p1) )
					{
						_imageScene->removeItem(r);
						_recognizedMarkers.removeAll(r);
						break;
					}
				 }

			}


			else if (m->state == MouseMarker::Released )
			{
				updateRegions();
			}
		}
		else if ( m->button == Qt::LeftButton )
		{
			switch(m->state)
			{
				case MouseMarker::Pressed:
					{
						if ( ui->bGetIcon->isChecked() )
						{
							foreach ( QGraphicsItem* r, _objectMarkers)
								_imageScene->removeItem(r);
						}
						else
						{
							_curMarker=_imageScene->addRect(m->p1.x(),m->p1.y(),0,0,QPen(currentClassCol()) );
						}
					}
					break;
				case MouseMarker::Released:
					{
						if ( ui->bGetIcon->isChecked() )
						{
							QPixmap icon = QPixmap (_project->_imgdb.getImagePath( _currentImage )).copy( m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y() );
							_project->setClassIcon( c ,icon );
							ui->bGetIcon->setChecked(false);
						}

						if(_project)
						{
														
							QPolygon p( QRect( m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y()));
							if (p.boundingRect().width() < 3)
							{
								if (_lastPolygon.boundingRect().width() < 3)
									return;
								p = _lastPolygon;// = QPolygon( QRect( m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y()));
								QRectF  r = p.boundingRect();
								QRect nr( 
									m->p1.x()-r.width()/2,
									m->p1.y()-r.height()/2,
									r.width(),
									r.height() );
								_curMarker=_imageScene->addRect( nr, QPen(currentClassCol()) );
								_project->addObject( c, _currentImage, QPolygon(nr) );
								_objectMarkers.append( _curMarker );
							}
							else
								_lastPolygon = p;

							if ( QFileInfo( _currentImage ).exists() ) //not image hash
								_currentImage = _project->addImage( _currentImage ); //TODO different vars for current ing and current hash


							_project->addObject( c, _currentImage, p );
							_objectMarkers.append( _curMarker );
						}

					}
					break;

				case MouseMarker::Moved:
					{
						if ( _curMarker ) 
							_imageScene->removeItem( _curMarker);

						_curMarker=_imageScene->addRect(m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y(),QPen(currentClassCol()) );
						
					}
					break;
			}
		}
	}
}


void MainWindow::on_bAcceptAllSuggested_clicked()
{
    QString c = currentClass( );
    if ( c.isEmpty() )
        return;

    foreach ( QGraphicsItem* r, _recognizedMarkers)
	{
		QGraphicsRectItem* rr =static_cast<QGraphicsRectItem*>(r);
		_project->addObject( c, _currentImage, QPolygon( rr->rect().toRect()) );
	}
	updateRegions();

}

void MainWindow::on_bRejectAllSuggested_clicked()
{
    foreach ( QGraphicsItem* r, _recognizedMarkers)
            _imageScene->removeItem(r);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	_curMarker(0)

{
    ui->setupUi(this);
	ui->lvImages->setModel(&_images);

	_imageScene = new ImageScene(this);
	ui->gvImage->setScene( _imageScene );

	

	//PERSISTENCE_INIT( "HEINITZ-IT.DE", "CellApp" );
	//PERSISTENT("LastProjectPath", &_currentProjectPath, this );

	//_currentProjectPath = 
	//openProject( _currentProjectPath );
	

	//loadDir("c:/temp/BloodCells/hematologyatlas.com/matureseq/");
	//loadDir("c:/temp/BloodCells/");

	ui->tvAtlas->setModel( &_atlasCells );

    
	_atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Erytrocyte.png")),"Erytrocyte" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Neutrophils.png")),"Neutrophils" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Monoytes.png")),"Monoytes" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Eosinophils.png")),"Eosinophils" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Lymphocytes.png")),"Lymphocytes" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Basophils.png")),"Basophils" )<< new QStandardItem("0" ));

	_mouseMarker = new MouseMarker( );
	_imageScene->setSelector( _mouseMarker );
	connect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
	loadPersistence();
}

MainWindow::~MainWindow()
{
	savePersistence();
    delete ui;
}


void MainWindow::on_actionExit_triggered()
{
    close();
}


void MainWindow::on_actionLoad_triggered()
{
    QString dn = QFileDialog::getExistingDirectory(this,tr("Select Dir"),_lastLoadedDir);
    if( dn.isEmpty() )
        return;

    loadDir(dn);
}

void MainWindow::loadDir(QString dn)
{
    QFileInfoList l = QDir(dn).entryInfoList( QStringList() <<"*.png"<<"*.jpg", QDir::Files );
    foreach ( QFileInfo fi, l )
    {
		_images.appendRow( new QStandardItem( fi.canonicalFilePath() ) );
    }
	_lastLoadedDir = dn;
}

void MainWindow::setSceneImage( QString img )
{
	_currentImage = img;
	QPixmap p( _project->_imgdb.getImagePath( _currentImage ) );
	_imageScene->setImage(p);	
	_recognizedMarkers.clear();
	_objectMarkers.clear();
}

void MainWindow::on_lvImages_clicked(const QModelIndex &index)
{
	setSceneImage( index.data().toString() );

	ui->gvImage->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ui->gvImage->fitInView( _imageScene->sceneRect(), Qt::KeepAspectRatio );

	if( ui->tabWidget->currentIndex() == 0 )
		;//analyse();


	//_imgOffset = QPoint( scene()->sceneRect().width() / width() , scene()->sceneRect().height() / height() );
	//_selector->setImageOffset( _imgOffset );
}

void MainWindow::on_lvImages_activated(const QModelIndex &index)
{

}

void MainWindow::analyse()
{
	foreach ( QGraphicsItem* m, _recognizedMarkers) _imageScene->removeItem(m); 

	QPixmap curimg( _currentImage );
	std::string img = FeatEx().load( _currentImage.toStdString(), "orig" );
	std::string green = FeatEx().separateChannel(img, 1 );

	FeatEx().op("cells", green, QString("%1,%2,0,6,7").arg(20).arg(60).toStdString(), "cellrects" );
	

	QList<QRect> crlls = FeatEx().rects( "cellrects" );

	_atlasCells.setData( _atlasCells.index(0,1), crlls.size() );

	img = FeatEx().normalize(img, 255 );

	FeatEx().op("backpj", "orig","100,100,20" );


	QPainter *paint = new QPainter(&curimg);
	paint->setPen(QColor(0,255,0,255));
	
	

	foreach( QRect cr,  crlls )
	{
		//paint->drawRect(cr);
		_recognizedMarkers << _imageScene->addRect( cr.x(),cr.y(),cr.width(),cr.height(),QPen(QColor("red")) );
	}
	delete paint;


	

#if 0			
				//img = FeatEx().separateChannel(img, 1 );
				//FeatEx().show(img);

				QList<double> res;
				res += FeatEx().extractObjectFeatures( img, "basic" );
				
				img = FeatEx().op( "bilateralFilter", img, "3,6,6" );
				img = FeatEx().normalize(img, 255 );
				res += FeatEx().extractObjectFeatures( img, "haralick" );
#endif			


}


int th= 100;
void MainWindow::on_bThadd_clicked()
{
	th+=10;
	std::string img = FeatEx().load(_currentImage.toStdString(), "orig" );
	img = FeatEx().op("canny", img, QString("%1,255,5").arg(th).toStdString(), "canny" );
	FeatEx().show(img,0);

}

void MainWindow::on_pThsub_clicked()
{
	th-=10;
	std::string img = FeatEx().load(_currentImage.toStdString(), "orig" );
	FeatEx().show(img,0);

	img = FeatEx().op("cells", img, "30,30,2,6,7", "cells" );
	FeatEx().show(img,0);
}

void MainWindow::on_actionClear_images_triggered()
{
    _images.clear();
}

void MainWindow::on_actionOpen_file_triggered()
{

}

void MainWindow::on_cbConfigType_currentIndexChanged(int index)
{
    ui->swConfigData->setCurrentIndex(index);
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
	
}

void MainWindow::on_actionOpen_Project_triggered()
{
	QString pf = QFileDialog::getOpenFileName( this, tr("Select Project File"), QFileInfo( _currentProjectPath ).canonicalPath() );

	if ( pf.isEmpty() )
	{
		return;
	}
	openProject( pf );

}

void MainWindow::on_actionNew_Project_triggered()
{
	QString newDir = QFileDialog::getExistingDirectory( this, tr("Select Directory") );

	if ( newDir.isEmpty() )
	{
		return;
	}

	_project = QSharedPointer<ProjectManager>( new ProjectManager());
	_project->create(  newDir, "CellApp.prj");
	_currentProjectPath = newDir + "/CellApp.prj";
	openProject( _currentProjectPath );
}

void MainWindow::openProject( QString fn )
{
	if (!QFileInfo(fn).exists()) return;

	_project = QSharedPointer<ProjectManager>( new ProjectManager() );
	_project->load(  fn );
	_currentProjectPath = fn;

	ui->lvModelImages->setModel( &_project->_modelImages );
	_modelImagesSelection = new QItemSelectionModel( &_project->_modelImages );
	ui->lvModelImages->setSelectionModel( _modelImagesSelection );

	ui->tvClasses->setModel( &_project->_classes );
	ui->tvClasses->setColumnWidth( CCIcon, ui->tvClasses->iconSize().width() );
	ui->tvClasses->setColumnWidth( CCColor, 25 );
	_classesSelection = new QItemSelectionModel( &_project->_classes );
	ui->tvClasses->setSelectionModel( _classesSelection );
	
	savePersistence();
}


void MainWindow::on_bAddClass_clicked()
{
	if(!_project) return;
	bool ok;
	QString name = QInputDialog::getText( this, "New Class", "Name:", QLineEdit::Normal, "", &ok );
	if (name.isEmpty()) return;

	_project->addClass( name );
}


void MainWindow::on_bRemoveClass_clicked()
{
	if(!_project) return;
	bool ok;

	_project->removeClass( currentClass( ) );
	updateRegions();
}


void MainWindow::on_lvModelImages_clicked(const QModelIndex &index)
{
	setSceneImage( index.data().toString() );

	ui->gvImage->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ui->gvImage->fitInView( _imageScene->sceneRect(), Qt::KeepAspectRatio );

	updateRegions();
}

void MainWindow::on_lvModelImages_activated(const QModelIndex &index)
{

}

void MainWindow::on_bSelectObjects_clicked(bool checked)
{
	if( checked )
	{
		connect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
	}
	else
	{
		//disconnect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
		//foreach ( QGraphicsRectItem* m, _objectMarkers) _imageScene->removeItem(m);
	}
}


void MainWindow::on_tvClasses_clicked(const QModelIndex &index)
{
	updateRegions();
}

void MainWindow::updateRegions()
{
	QString c = currentClass( );
	if ( c.isEmpty() )
		return;

	if ( _currentImage.isEmpty() )
		return;

	
	foreach( QGraphicsItem* i, _objectMarkers )
	{
		_imageScene->removeItem(i);
	}
	_objectMarkers.clear();

	foreach( ImageObject i, _project->_objects )
	{
		if( i._imghash == _currentImage )
		{
			if ( c.isEmpty() )
				_objectMarkers << _imageScene->addPolygon(i._region,QPen(_project->classColor( i._class )) );
			else if ( c == i._class )
				_objectMarkers << _imageScene->addPolygon(i._region,QPen(_project->classColor( i._class )) );
		}
	}
}

void MainWindow::on_tvClasses_doubleClicked(const QModelIndex &index)
{
	if ( index.column( ) == CCColor ) //color
	{
		QString c = index.model()->index(index.row(), CCName).data().toString(); 
		QColor col = QColorDialog::getColor( QColor(index.data(Qt::DecorationRole).toString()), this, tr("Select color") );
		_project->setClassColor( c, col );
	}
}

void MainWindow::on_bSuggest_clicked()
{
	// Collect statistics on class
	QString c = currentClass( );
	if ( c.isEmpty() )
		return;

	if ( _currentImage.isEmpty() )
		return;

	int minedge=99999999;
	int maxedge=0;
	foreach( ImageObject i, _project->_objects )
	{
		//if( i._imghash == _currentImage )
		{
			if ( c == i._class )
			{
				QRect r = i._region.boundingRect();
				minedge = qMin(minedge,qMin(r.width(),r.height()));
				maxedge = qMax(maxedge,qMax(r.width(),r.height()));
			}
		}
	}

	std::string img = FeatEx().load( _project->_imgdb.getImagePath( _currentImage ).toStdString(), "orig" );
	std::string green = FeatEx().separateChannel(img, 1 );
	//FeatEx().op("seg", green );

	FeatEx().op("cells", green, QString("%1,%2,1,6,7").arg(minedge*0.8).arg(maxedge*1.2).toStdString(), "cellrects" );
	

	QList<QRect> crlls = FeatEx().rects( "cellrects" );
	foreach( QRect cr,  crlls )
	{
		_recognizedMarkers << _imageScene->addRect( cr.x(),cr.y(),cr.width(),cr.height(),QPen(QColor("blue")) ); // get from settings
	}

}


void MainWindow::on_bLearnModel_clicked()
{
	_project->learn();
}

void MainWindow::on_bRemoveModelInstance_clicked()
{

}
