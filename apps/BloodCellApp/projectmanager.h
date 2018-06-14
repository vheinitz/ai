#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QString>
#include "db/imgdb.h"
#include <QStandardItemModel>
#include <QPolygon>

struct ImageObject
{
	QString _class;
	QString _imghash;
	QPolygon _region;
	int _uid;
};

enum ClassColums{ CCIcon=0, CCColor, CCName };

class ProjectManager
{
public:
	enum ClasModelRoles{ ClassIconFile=Qt::UserRole+1 };
    ProjectManager( QString fn = QString::null);
	~ProjectManager();
    QString _dir;
	QString _fn;
    void load( QString fn);
	void save( );
	void addClass( QString c );
	QModelIndex classIndex( QString c );
	QModelIndex classColorIndex( QString c );
	QColor classColor( QString c );
	QModelIndex classIconIndex( QString c );
	void setClassIcon( QString c, QPixmap icon );
	void setClassColor( QString c, QColor col );
	void addObject( QString c, QString imghash, QPolygon r );
	void removeAt( QString c, QString imghash, QPoint p );
	void create( QString dir, QString fn);
	QString addImage( QString fn );
	ImageDatabase _imgdb;
	QStandardItemModel _classes;
	QStandardItemModel _modelImages;
	QList<ImageObject> _objects;
};

#endif // PROJECTMANAGER_H
