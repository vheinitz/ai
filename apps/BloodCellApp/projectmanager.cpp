#include "projectmanager.h"
#include <QFileInfo>
#include "os/fstools.h"
#include <QDir>
#include <QUuid>

ProjectManager::ProjectManager(QString fn)
{
    if (!fn.isEmpty()) load(fn);
}

ProjectManager::~ProjectManager( )
{
    if (!_fn.isEmpty()) save();
}

QColor randomColor()
{
    int red = int(185 + 70.0*qrand()/(RAND_MAX+1.0));
    int green = int(185 + 70.0*qrand()/(RAND_MAX+1.0));
    int blue = int(205 + 50.0*qrand()/(RAND_MAX+1.0));
    //int alpha = int(91 + 100.0*qrand()/(RAND_MAX+1.0));

    return QColor(red, green, blue, 255);
}

void ProjectManager::load( QString fn)
{
	_dir = QFileInfo(fn).canonicalPath();
	_fn = QFileInfo(fn).fileName();
	_imgdb.setRoot(_dir+"/imagedata");
	QDir().mkpath(_dir+"/icons");
	foreach(QString h, _imgdb.images() )
	{
		_modelImages.appendRow(QList<QStandardItem*>() << new QStandardItem( QIcon(QPixmap(_imgdb.getThumbnailPath(h))),h ));
	}

	QStringList classes = FSTools::fromFile( _dir+"/"+_fn );
	foreach(QString c, classes) {
		QStringList items = c.split( ";" );
		if (items.size() < 2)
			continue;
		if( items.at(0).trimmed() == "CLASS" )
		{
			addClass(items.at(1));
			if( items.size()>2 )
			{
				QPixmap p = QPixmap(items.at(2));
				setClassIcon( items.at(1),items.at(2) );
			}
			if( items.size()>3 )
			{
				QColor p(items.at(3));
				setClassColor( items.at(1),items.at(3) );
			}
		}

		else if( items.at(0).trimmed() == "OBJECT" )
		{
			ImageObject i;
			i._class = items.at(1);
			i._imghash = items.at(2);
			QVector<QPoint> points;
			foreach( QString p, items.at(3).split(",") )
			{
				QStringList xy = p.split(" ");
				if (xy.size() != 2)
					continue;

				points << QPoint( xy.at(0).toInt(), xy.at(1).toInt() );
			}
			i._region = QPolygon( points );
			_objects << i;
		}   
    }
}

void ProjectManager::save( )
{
	QStringList data;
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex cindex = _classes.index(r, CCColor);
		QModelIndex iindex = _classes.index(r, CCIcon);
		QModelIndex index = _classes.index(r, CCName);
		data << QString ("CLASS;%1;%2;%3")
			.arg(_classes.data(index).toString())
			.arg(_classes.data(iindex,ProjectManager::ClassIconFile).toString())
			.arg(_classes.data(cindex,Qt::DecorationRole).toString());
    }

	foreach(ImageObject i, _objects) {
  
		QStringList reg;
		foreach( QPoint p, i._region.toList() )
		{
			reg << QString( "%1 %2" ).arg(p.x()).arg(p.y());
		}

		data << QString ("OBJECT;%1;%2;%3")
			.arg(i._class)
			.arg(i._imghash)
			.arg(reg.join(","));
    }

	FSTools::toFile( data, _dir+"/"+_fn );
}


QModelIndex ProjectManager::classIndex( QString c )
{
	QModelIndex index = _classes.index(-1, -1);
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex tmp = _classes.index(r, CCName);
        if ( _classes.data(tmp).toString() == c)
		{
			index = tmp;
			break;
		}
    }

	return index;
}

QModelIndex ProjectManager::classColorIndex( QString c )
{
	QModelIndex index = _classes.index(-1, -1);
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex tmp = _classes.index(r, CCName);
        if ( _classes.data(tmp).toString() == c)
		{
			index = _classes.index(r, CCColor);
			break;
		}
    }

	return index;
}

QColor ProjectManager::classColor( QString c )
{
	QModelIndex index = _classes.index(-1, -1);
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex tmp = _classes.index(r, CCName);
        if ( _classes.data(tmp).toString() == c)
		{
			index = _classes.index(r,CCColor);
			break;
		}
    }

	if (index.isValid())
		return QColor( index.data(Qt::DecorationRole).toString() );
	return
		QColor();
}

QModelIndex ProjectManager::classIconIndex( QString c )
{
	QModelIndex index = _classes.index(-1, -1);
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex tmp = _classes.index(r, CCName);
        if ( _classes.data(tmp).toString() == c)
		{
			index = _classes.index(r, CCIcon);
			break;
		}
    }

	return index;
}
 

void ProjectManager::addClass( QString c )
{
	QModelIndex index = classIndex( c );
	if ( index.isValid() )
		return;

	QStandardItem *col = new QStandardItem( "   " );
	col->setData( randomColor(), Qt::DecorationRole );

	_classes.appendRow(QList<QStandardItem*>() 
		<< new QStandardItem(  )
		<< col
		<< new QStandardItem( c )		
		 );
}

QStringList ProjectManager::classes( )
{
	QStringList cl;

	for(int i=0; i<_classes.rowCount(); i++)
	{
		cl<< _classes.data( _classes.index(i,CCName) ).toString();
	}
	
	return cl;
}

QStringList ProjectManager::modelImages( )
{
	QStringList mi;

	for(int i=0; i<_modelImages.rowCount(); i++)
	{
		mi<< _modelImages.data( _classes.index(i,0) ).toString();
	}
	
	return mi;
}

void ProjectManager::removeClass( QString c )
{
	QModelIndex index = classIndex( c );
	if ( index.isValid() )
	{
		setClassIcon( c ); 
		for( int idx = 0; idx < _objects.size(); idx++) 
		{
			ImageObject i = _objects.at(idx);
			if ( i._class == c )
			{
				_objects.removeAt(idx);
				idx=0;
			}
		}
	}
}

void ProjectManager::setClassIcon( QString c, QPixmap icon )
{
	QModelIndex index = classIconIndex( c );
	if ( !index.isValid() )
		return;

	QString iconpath = _classes.data(index,ProjectManager::ClassIconFile).toString();
	QFile::remove(iconpath);

	if ( icon.isNull() )
		return;

	QString iconhash = FSTools::getHash( icon );
	iconpath = _dir+"/icons/"+iconhash+".png";
	bool ok = icon.save(iconpath);
	_classes.setData(index,icon.scaled( 50,50 ),Qt::DecorationRole);
	_classes.setData(index,iconpath,ProjectManager::ClassIconFile);
}

void ProjectManager::setClassColor( QString c, QColor col )
{
	QModelIndex index = classColorIndex( c );
	if ( !index.isValid() )
		return;
	_classes.setData(index,col,Qt::DecorationRole);
}


QString ProjectManager::addImage( QString fn)
{
	bool existed;
	QString hash = _imgdb.addImage( fn, &existed );
	if(!existed)
		_modelImages.appendRow(QList<QStandardItem*>() << new QStandardItem( QIcon(QPixmap(_imgdb.getThumbnailPath(hash))),hash ));

	return hash;
}

void ProjectManager::removeImage( QString hash)
{

	_imgdb.removeImage( hash );

	for(int r=0; r<_modelImages.rowCount(); r++ )
	{
		 
		if( _modelImages.data( _modelImages.index( r, 0 ) ).toString() == hash )
		{
			_modelImages.removeRow(r);
			break;
		}
	}

	for( int i = 0; i<_objects.size(); i++ )
	{
		ImageObject o = _objects.at(i);
		if(  o._imghash == hash  )
		{
			_objects.removeAt(i);
			i=0;
		}
	}
}

void ProjectManager::addObject( QString c, QString imghash, QPolygon r )
{
	static int uid = 0;
	ImageObject obj;
	obj._class = c;
	obj._imghash = imghash;
	obj._region = r;
	obj._uid = ++uid;

	_objects << obj;
}

QPair<int,int> ProjectManager::objectSizeMinMax(QString c, QString img)
{
	int minedge=99999999;
	int maxedge=0;
	
	foreach( ImageObject i, _objects )
	{
		if( img.isEmpty() || i._imghash == img )
		{
			if ( c == i._class )
			{
				QRect r = i._region.boundingRect();
				minedge = qMin(minedge,qMin(r.width(),r.height()));
				maxedge = qMax(maxedge,qMax(r.width(),r.height()));
			}
		}
	}
	return QPair<int,int>(minedge,maxedge);
}

void ProjectManager::removeAt( QString c, QString imghash, QPoint p )
{
	
	for( int i = 0; i<_objects.size(); i++ )
	{
		ImageObject o = _objects.at(i);
		if( o._class == c && o._imghash == imghash && o._region.boundingRect().contains(p) )
		{
			_objects.removeAt(i);
		}
	}
}

ImageObject ProjectManager::objectAt( QString c, QString imghash, QPoint p )
{
	for( int i = 0; i<_objects.size(); i++ )
	{
		ImageObject o = _objects.at(i);
		if( o._class == c && o._imghash == imghash && o._region.boundingRect().contains(p) )
		{
			return o;
		}
	}
	return ImageObject();
}

void ProjectManager::learn(  )
{
	int classId=0;
	QMap<QString,int> cm;
	foreach( QString c, classes() )
	{
		if ( !cm.contains(c) )
		  cm[c] = classId++;
	}

	QMap<int, QMap< QString, QList<QRect> > > cl_img_rects;

	for( int i = 0; i<_objects.size(); i++ )
	{
		ImageObject o = _objects.at(i);
		cl_img_rects[ cm[o._class] ][ o._imghash ].append( o._region.boundingRect() );
	}

	QStringList cl = classes();
	QStringList mi = modelImages();

	QMap<QString, QStringList> modelData;

	foreach( int c, cm )
	{
		foreach( QString img, cl_img_rects[c].keys() )
		{
			QImage p =  _imgdb.getImage( img ) ;

			int maxpv =0;
			for (int ii = 0; ii < p.height(); ii++) {
					uchar* scan = p.scanLine(ii);
					int depth =4;
					for (int jj = 0; jj < p.width(); jj++) {

						QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + jj*depth);
						int g = qGray(*rgbpixel);
						maxpv = qMax( g,maxpv );
					}
				}
			double normfactor = 255. / maxpv;

			foreach( QRect r, cl_img_rects[c][img] )
			{
				QStringList imgdata;
				imgdata << QString::number(c);
				QImage roi = p.copy( r ).scaled(32,32);
				//roi.save( QString( "c:/tmp/bc/%1_%2%3%4%5.png" ).arg( img )
				//	.arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()) );

				for (int ii = 0; ii < roi.height(); ii++) {
					uchar* scan = roi.scanLine(ii);
					int depth =4;
					for (int jj = 0; jj < roi.width(); jj++) {

						QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + jj*depth);
						imgdata << QString::number( (int)(qGray(*rgbpixel) * normfactor) );
					}
				}
				modelData[QUuid::createUuid().toString()]= imgdata;
			}
		}
	}

	int i=0;
	QList<QStringList> d;
	foreach( QStringList l, modelData.values() )
	{
			d<<l;
	}

	FSTools::toFile( d, " ", "c:/tmp/bc/data" );

}

#include <QLabel>
#include <QApplication>

void ProjectManager::test( QString imgid, QString cn )
{
	QLabel l;
	foreach( QString c, classes() )
	{
		if( !cn.isEmpty() )
			c = cn;
		QList< QList<int> > data;
		QPair<int,int> csize = objectSizeMinMax( c);
		int edge = csize.second;
		QImage img =  _imgdb.getImage( imgid ) ;

		int maxpv =0;
		for (int ii = 0; ii < img.height(); ii++) {
				uchar* scan = img.scanLine(ii);
				int depth =4;
				for (int jj = 0; jj < img.width(); jj++) {

					QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + jj*depth);
					int g = qGray(*rgbpixel);
					maxpv = qMax( g,maxpv );
				}
			}
		double normfactor = 255. / maxpv;
		
        int step = edge/4;
		for ( int x=0; x<img.width()-edge; x+=step )
		{
			for ( int y=0; y<img.height()-edge; y+=step )
			{
				QList<int> linedata;
				linedata << 0; // Class
				//QImage r =  img.copy(x,y,edge,edge);
				//r.save( QString("c:/tmp/bc/t/%1_%2_%3.png").arg( c ).arg( x ).arg( y ) );


				for (int ii = y; ii < y + edge; ii++) {
					uchar* scan = img.scanLine(ii);
					int depth =4;
					for (int jj = x; jj < x+ edge; jj++) {

						QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + jj*depth);
						int g = qGray(*rgbpixel * normfactor);
						linedata << g;
					}
				}
				data << linedata;
				//FSTools::toCsv( data, QString("c:/tmp/bc/test_%1.txt").arg(c), ' ' );
			}
		}
		FSTools::toCsv( data, QString("c:/tmp/bc/test_%1.txt").arg(c), ' ' );
		if( !cn.isEmpty() )
			break; 
	}	
}

void ProjectManager::create( QString dir, QString fn)
{
	_dir = dir;
	_fn=fn;
	QDir().mkpath(dir);
	QDir().mkpath(dir+"/icons");
	FSTools::touch( dir+"/"+fn );

	load( dir+"/"+fn );
}

