#ifndef FEATURE_EXTRACTOR_HG__H
#define FEATURE_EXTRACTOR_HG__H

#include <QSharedPointer>
#include <list>
#include <string>
#include <QVariant>


class FeatEx_p;

#ifdef BUILDING_FEATEX_DLL
# define FEATEX_EXPORT Q_DECL_EXPORT
#else
# define FEATEX_EXPORT Q_DECL_IMPORT
#endif

#ifdef BUILDING_FEATEX_EMBEDDED
# undef FEATEX_EXPORT
# define FEATEX_EXPORT
#endif

class FEATEX_EXPORT FeatEx
{

public:
    FeatEx();

	std::string load( std::string fn, std::string obj=std::string() ); ///Load from file, type is as defined in file
	std::string normalize( std::string src, int range, std::string dst=std::string() );
	std::string crop( std::string src, int range, std::string dst );
	std::string separateChannel( std::string src, int ch, std::string dst=std::string() );
	std::string blur( std::string src, int dotsize, std::string dst=std::string() );
	std::string op( std::string op, std::string src, std::string param=std::string(), std::string dst=std::string() );
	QList<double> extractObjectFeatures(  std::string obj, std::string featureSet, QVariant param = QVariant() ); 
	QStringList fnames( std::string featureSet, std::string prefix=std::string() );
	void show(std::string img, bool pause=true);
	
private:
   QSharedPointer<FeatEx_p> _p;
	
};

#endif // FEATURE_EXTRACTOR_HG__H
