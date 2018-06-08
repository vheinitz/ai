#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QString>
#include "db/imgdb.h"
#include <QStandardItemModel>



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
	void setClassIcon( QString c, QPixmap icon );
	void create( QString dir, QString fn);
	void addImage( QString fn );
	ImageDatabase _imgdb;
	QStandardItemModel _classes;
	QStandardItemModel _modelImages;
};

#endif // PROJECTMANAGER_H
