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
		}


        
    }

	
}

void ProjectManager::save( )
{
	QStringList data;
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex index = _classes.index(r, 0);
		data << QString ("CLASS;%1;%2").arg(_classes.data(index).toString()).arg(_classes.data(index,ProjectManager::ClassIconFile).toString());
    }

	FSTools::toFile( data, _dir+"/"+_fn );
}


QModelIndex ProjectManager::classIndex( QString c )
{
	QModelIndex index = _classes.index(-1, -1);
	for(int r = 0; r < _classes.rowCount(); ++r) {
        QModelIndex tmp = _classes.index(r, 0);
        if ( _classes.data(tmp).toString() == c)
		{
			index = tmp;
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

	_classes.appendRow(QList<QStandardItem*>() << new QStandardItem( c ) );
}

void ProjectManager::setClassIcon( QString c, QPixmap icon )
{
	QModelIndex index = classIndex( c );
	if ( !index.isValid() )
		return;
	QString iconhash = FSTools::getHash( icon );
	QString iconpath = _dir+"/icons/"+iconhash+".png";
	bool ok = icon.save(iconpath);
	_classes.setData(index,icon,Qt::DecorationRole);
	_classes.setData(index,iconpath,ProjectManager::ClassIconFile);
}

void ProjectManager::addImage( QString fn)
{
	bool existed;
	QString hash = _imgdb.addImage( fn, &existed );
	if(!existed)
		_modelImages.appendRow(QList<QStandardItem*>() << new QStandardItem( QIcon(QPixmap(_imgdb.getThumbnailPath(hash))),hash ));
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


void ;

void ProjectManager::create( QString dir, QString fn)
{
	_dir = dir;
	_fn=fn;
	QDir().mkpath(dir);
	QDir().mkpath(dir+"/icons");
	FSTools::touch( dir+"/"+fn );

	load( dir+"/"+fn );
}