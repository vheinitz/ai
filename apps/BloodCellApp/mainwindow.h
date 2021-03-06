#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <gui/imageview.h>
#include <QSharedPointer>
#include <QItemSelectionModel>
#include <Qt>


class MouseMarker: public QObject, public ObjectSelectorInterface 
{
Q_OBJECT
public:
  QPoint p1;
  QPoint p2;
  enum State{None=0, Pressed, Moved, Released};
  State state;
  Qt::MouseButton button;
  MouseMarker( );
  void mousePressed( QPoint p, Qt::MouseButton b = Qt::NoButton );
  void mouseReleased( QPoint p );
  void mouseMoved( QPoint );
signals:
  void check();
};


class ProjectManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	QString currentClass( );
	int currentClassIdx( );
	QColor currentClassCol( );

	void setSceneImage( QString  );

private slots:
	void checkCellMarker(  );
	void on_actionExit_triggered();
    void on_actionLoad_triggered();
	void loadDir(QString dn);

    void on_lvImages_clicked(const QModelIndex &index);

    void on_lvImages_activated(const QModelIndex &index);

    void analyse();

    void on_bThadd_clicked();

    void on_pThsub_clicked();

    void on_actionClear_images_triggered();

    void on_actionOpen_file_triggered();

    void on_cbConfigType_currentIndexChanged(int index);

    void on_tabWidget_currentChanged(int index);

    void on_actionOpen_Project_triggered();

    void on_actionNew_Project_triggered();

	void openProject( QString );

    void on_bAddClass_clicked();

	void savePersistence();
	void loadPersistence();

    void on_lvModelImages_clicked(const QModelIndex &index);

    void on_lvModelImages_activated(const QModelIndex &index);

    void on_bSelectObjects_clicked(bool checked);

    void on_tvClasses_clicked(const QModelIndex &index);

	void updateRegions();

    void on_tvClasses_doubleClicked(const QModelIndex &index);

    void on_bSuggest_clicked();

    void on_bAcceptAllSuggested_clicked();

    void on_bRejectAllSuggested_clicked();

    void on_bRemoveClass_clicked();

    void on_bRemoveModelInstance_clicked();

    void on_bLearnModel_clicked();
	void on_bTest_clicked();

    void on_bDevTest_clicked();

private:
    Ui::MainWindow *ui;

	QStandardItemModel _images;
    QStandardItemModel _atlasCells;
	ImageScene *_imageScene;
	QList<double> _meanRbcDiam;
	//QList<QGraphicsItem*> _lineMarkers;
	QList<QGraphicsItem*> _recognizedMarkers; //marks by recognized
	QList<QGraphicsItem*> _objectMarkers;       //Marks of used-selected objects
	QGraphicsItem* _curMarker;
	QString _currentProjectPath;
	QString _currentImage;
	QString _lastLoadedDir;
    QSharedPointer<ProjectManager> _project;
	MouseMarker *_mouseMarker;
	QItemSelectionModel *_classesSelection;
	QItemSelectionModel *_modelImagesSelection;
};




#endif // MAINWINDOW_H
