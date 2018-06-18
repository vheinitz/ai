#include "projectmanager.h"
#include <QFileInfo>
#include "os/fstools.h"
#include <QDir>

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

void ProjectManager::removeClass( QString c )
{
	QModelIndex index = classIndex( c );
	if ( index.isValid() )
	{
		_classes.removeRow(index.row()); 

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
	QString iconhash = FSTools::getHash( icon );
	QString iconpath = _dir+"/icons/"+iconhash+".png";
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


void ProjectManager::create( QString dir, QString fn)
{
	_dir = dir;
	_fn=fn;
	QDir().mkpath(dir);
	QDir().mkpath(dir+"/icons");
	FSTools::touch( dir+"/"+fn );

	load( dir+"/"+fn );
}