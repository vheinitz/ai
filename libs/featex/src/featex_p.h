#ifndef FEATURE_EXTRACTOR_P_HG__H
#define FEATURE_EXTRACTOR_P_HG__H

#include <QImage>
#include <QMutex>
#include <QMap>
#include <QQueue>
#include <QStringList>
#include <QSharedPointer>
#include <QVariant>
#include <cells.h>

#include <QFuture>
#include <QFutureWatcher>



/* TODO: describe features as JSON
{ "FID": "Feature-ID", "FV": "Version", "IOP": [ ["operation1","av1"],["operation2","av1","av2","avN"] ], 
"OS":[["os1","av1","av2","av3"],["os1","av1"]], 
"OOP":[["oop1","av1","av2","av3"],["oop","av1","av2"]], 
"FA":["av1","av2","avN"] }

TODO cache all intermediate images.
Caching Images:
(Img, OP1 ) -> Image
(Img, OP1, OP2 ) -> Image
(Img, OP1, OP3,...OPN ) -> Image

Caching Objects:
(Img, <Image OPs>, <Object Selectors>, <ObjectOps>  )

*/

class FeatureSetResults
{
public:
	QList< QFutureWatcher<double>* > _inExtraction;
	QStringList _featureSet;
};

typedef QSharedPointer<FeatureSetResults> TPFeatureSetResults;


class  FeatEx_p : public QObject
{
    Q_OBJECT

	QMap<QString,double> _cache;
public:
    FeatEx_p(QObject *parent = 0);
    ~FeatEx_p();

	QList<QRect> rects(  std::string obj ); 
	QList<double> extractObjectFeatures(  std::string obj, std::string featureSet, QVariant param = QVariant()  ); 
	QStringList fnames( std::string featureSet, std::string prefix = std::string() );
	void show(std::string img, bool pause=true);

	std::string load( std::string fn, std::string obj ); ///Load from file, type is as defined in file
	std::string loadGrey( std::string fn, std::string obj ); ///Load from file, type is grey
	std::string crop( std::string src, int range, std::string dst );
	std::string normalize( std::string src, int range,std::string dst );
	std::string separateChannel( std::string src, int ch, std::string dst );
	std::string blur( std::string src, int dotsize, std::string dst );
	std::string op( std::string op, std::string src, std::string param, std::string dst );

	
private:

	QList<double> extractBasicFeatures( std::string objid  );
	QList<double> extractHaralickFeatures( std::string objid  );
	QList<double> extractHistogramFeatures( std::string objid  );
	QList<double> extractMat( std::string objid, int size  );
	QList<double> extractHOG(  std::string objid  );
	
   
	bool _stop;
	QMap<QString, TPFeatureSetResults > _featureSets;
	QMutex _featureSetsMx;
	
	QMutex _deleteMx;
	int _futureCnt;
	QMap<int, QFuture<double> > _runningFutures;
	QMutex _runningFuturesMx;

	
};

#endif // FEATURE_EXTRACTOR_HG__H
