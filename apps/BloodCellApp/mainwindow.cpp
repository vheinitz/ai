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


class ProjectManager;



MouseMarker::MouseMarker( ){};
void MouseMarker::mousePressed( QPoint p )
{
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


void MainWindow::checkMarker( )
{
	MouseMarker *m = qobject_cast<MouseMarker*>( sender() );
	if(m)
	{
		switch(m->state)
		{
			case MouseMarker::Pressed:
				{
					FeatEx().op("backpj", "orig", QString("%1,%2,10").arg( m->p1.x()).arg( m->p1.y() ).toStdString(),"backpj" );
					FeatEx().op("add", "backpj", "bpsum", "bpsum" );
					FeatEx().op("show", "bpsum" );
					//ui->bMarkRBC->setChecked(false);
					_curLineMarker=_imageScene->addLine(m->p1.x(),m->p1.y(),m->p2.x(),m->p2.y(),QPen(QColor("red")) );
				}
				break;
			case MouseMarker::Released:
				{
					//ui->bMarkRBC->setChecked(false);
					//_imageScene->removeItem( _curLineMarker);
					_lineMarkers << _curLineMarker;
					double len = qAbs( 
											qSqrt( 
													qPow(m->p1.x() - m->p2.x(),2) 
													+
													qPow(m->p1.y() - m->p2.y(),2)
												)
											);

					_meanRbcDiam <<  len;
					
					double avg=0;
					foreach( double d, _meanRbcDiam ){ avg+=d; }
					avg /=_meanRbcDiam.size();

					ui->eRbcSize->setText( QString::number(qRound(avg)) );
					if(_project)_project->addImage( _currentImage );
				}
				break;
			case MouseMarker::Moved:
				{
					_imageScene->removeItem( _curLineMarker);
					_curLineMarker=_imageScene->addLine(m->p1.x(),m->p1.y(),m->p2.x(),m->p2.y(),QPen(QColor("red")) );
					
				}
				break;
		}
	}
}


void MainWindow::checkCellMarker( )
{
	QString currentClass;
	if ( !_classesSelection->selection().isEmpty() )
	{
		QModelIndex ix = _classesSelection->selectedIndexes().first();
		currentClass = ix.data().toString();
	}
	else
		return; // no class selected

	MouseMarker *m = qobject_cast<MouseMarker*>( sender() );
	if(m)
	{
		switch(m->state)
		{
			case MouseMarker::Pressed:
				{
					if ( ui->bREmoveObjects->isChecked() )
					{
						foreach ( QGraphicsRectItem* r, _cellMarkers)
							if( r->contains(m->p1) )
								_imageScene->removeItem(r);
					}
					else
					{
						_curCellMarker=_imageScene->addRect(m->p1.x(),m->p1.y(),0,0,QPen(QColor("red")) );
					}
				}
				break;
			case MouseMarker::Released:
				{
					if ( ui->bGetIon->isChecked() )
					{
						QPixmap icon = QPixmap (_currentImage).copy( m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y() );
						_project->setClassIcon( currentClass ,icon );
					}


					if(_project)
					{
						_project->addImage( _currentImage );
						_project->addObject( currentClass, _currentImage, QPolygon( QRect( m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y())));
					}

				}
				break;
			case MouseMarker::Moved:
				{
					_imageScene->removeItem( _curCellMarker);
					_curCellMarker=_imageScene->addRect(m->p1.x(),m->p1.y(),m->p2.x()-m->p1.x(),m->p2.y()-m->p1.y(),QPen(QColor("red")) );
					
				}
				break;
		}
	}
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->lvImages->setModel(&_images);

	_imageScene = new ImageScene(this);
	ui->gvImage->setScene( _imageScene );

	_mouseMarker = new MouseMarker( );
	_imageScene->setSelector( _mouseMarker );

	//PERSISTENCE_INIT( "HEINITZ-IT.DE", "CellApp" );
	//PERSISTENT("LastProjectPath", &_currentProjectPath, this );

	//_currentProjectPath = 
	//openProject( _currentProjectPath );
	

	//loadDir("c:/temp/BloodCells/hematologyatlas.com/matureseq/");
	loadDir("c:/temp/BloodCells/");

	ui->tvAtlas->setModel( &_atlasCells );

    
	_atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Erytrocyte.png")),"Erytrocyte" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Neutrophils.png")),"Neutrophils" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Monoytes.png")),"Monoytes" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Eosinophils.png")),"Eosinophils" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Lymphocytes.png")),"Lymphocytes" )<< new QStandardItem("0" ));
    _atlasCells.appendRow(QList<QStandardItem*>() << new QStandardItem(QIcon(QPixmap(":/res/Basophils.png")),"Basophils" )<< new QStandardItem("0" ));

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

void MainWindow::on_lvImages_clicked(const QModelIndex &index)
{
	_currentImage = index.data().toString();
		
	QPixmap p( _currentImage );
	_imageScene->setImage(p);

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
	foreach ( QGraphicsItem* m, _rectMarkers) _imageScene->removeItem(m); 

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
		_rectMarkers << _imageScene->addRect( cr.x(),cr.y(),cr.width(),cr.height(),QPen(QColor("red")) );
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

	ui->tvClasses->setModel( &_project->_classes );
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

void MainWindow::on_bGetIon_clicked()
{
	connect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
}

void MainWindow::on_lvModelImages_clicked(const QModelIndex &index)
{
	_currentImage = index.data().toString();
		
	bool e = QFileInfo( _currentImage ).exists();
	QPixmap p( _project->_imgdb.getImagePath( _currentImage ) );
	_imageScene->setImage(p);

	ui->gvImage->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ui->gvImage->fitInView( _imageScene->sceneRect(), Qt::KeepAspectRatio );

	if( ui->tabWidget->currentIndex() == 0 )
		;//analyse();
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
		disconnect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
		//foreach ( QGraphicsRectItem* m, _cellMarkers) _imageScene->removeItem(m);
	}
}

void MainWindow::on_bREmoveObjects_clicked(bool checked)
{
	if( checked )
	{
		connect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
	}
	else
	{
		disconnect( _mouseMarker, SIGNAL(check()), this , SLOT( checkCellMarker(  ) ) );
	}
}
