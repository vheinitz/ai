#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <featex.h>
#include <os/fstools.h>
#include <QFileInfo>
#include <QDir>
#include <QDebug>


int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	bool append=false;
	bool colNames=false;
	QString classCol;
	QString rowIdType;
	int verboseLevel=0;
	int limit=999999999;
	QString csvFile = "c:/tmp/featex.csv", csvLabels, csvData, scrFn;
	QString indir;
	
	foreach( QString arg, app.arguments() )
	{
		if (arg.contains("--indir=") )
		{
			indir = arg.section("=",1);
		}
		else if (arg.contains("--csv=") )
		{
			csvFile = arg.section("=",1);
		}
		else if (arg.contains("--lcsv=") )
		{
			csvLabels = arg.section("=",1);
		}
		else if (arg.contains("--dcsv=") )
		{
			csvData = arg.section("=",1);
		}
		else if (arg.contains("--scr=") )
		{
			scrFn = arg.section("=",1);
		}
		else if (arg == "--append") 
		{
			append = true;
		}
		else if (arg == "--colNames") 
		{
			colNames = true;
		}
		else if (arg.contains("--classCol=")) 
		{
			classCol = arg.section("=",1);
		}
		else if (arg.contains("--verbose=")) 
		{
			verboseLevel = arg.section("=",1).toInt();
		}
		else if (arg.contains("--limit=")) 
		{
			limit = arg.section("=",1).toInt();
		}
		else if (arg.contains("--rowId=")) 
		{
			rowIdType = arg.section("=",1).toInt();
		}
	}

	if (!QFileInfo(indir).exists())
		return 2;

	if (!QFileInfo(scrFn).exists())
		return 2;

	

	QTextStream *fs;
	if ( append )
		fs = new TAStream( csvFile );
	else
		fs = new TOStream( csvFile );

	/*bool addSep=false;
	if(colNames)
	{

		if ( !rowIdType.isEmpty() )
		{
			if (addSep) *fs << ',';
			*fs <<"Row ID";
			addSep=true;
		}

		if ( !classCol.isEmpty() )
		{
			if (addSep) *fs << ',';
			*fs <<"ClassId";
			addSep=true;
		}
		

		QStringList fnames;
		fnames += FeatEx().fnames( "basic" );
		fnames += FeatEx().fnames( "haralick" );
		fnames += FeatEx().fnames( "histogram", "1" );
		fnames += FeatEx().fnames( "histogram", "2" );
		fnames += FeatEx().fnames( "histogram", "3" );
		fnames += FeatEx().fnames( "histogram", "4" );
		fnames += FeatEx().fnames( "histogram", "M_" );
		fnames += FeatEx().fnames( "histogram", "a_" );
		
		fnames += FeatEx().fnames( "hog", "1" );
		fnames += FeatEx().fnames( "hog", "2" );
		fnames += FeatEx().fnames( "hog", "3" );
		fnames += FeatEx().fnames( "hog", "4" );
		

		//fnames += FeatEx().fnames( "mat32x32" );
		//fnames += FeatEx().fnames( "mat4x4" );
		fnames += FeatEx().fnames( "mat8x8" );
		//fnames += FeatEx().fnames( "mat16x16" );
		foreach (QString cn, fnames)
		{
			if (addSep) *fs << ',';
			*fs << cn;
		}
		*fs<<"\n";
	}
	*/

	QStringList scr = FSTools::fromFile(scrFn);

	QFileInfoList files = QDir(indir).entryInfoList(QDir::Files);

	int cnt=0;
	foreach( QFileInfo fi, files )
	{
		try
		{
			std::string fn = fi.canonicalFilePath().toStdString();
			std::string img = FeatEx().load(fn );

			QList<double> res;
			foreach( QString instr, scr )
			{

				QStringList i = instr.split(" ");
				if (i.size() < 1) continue;
				if (i.at(0) == "X")
				{
					res += FeatEx().extractObjectFeatures( img, i.at(1).toStdString() );
				}
				else if (i.at(0) == "O")
				{
					std::string params = i.size()>2? i.at(2).toStdString() : "";
					img = FeatEx().op( i.at(1).toStdString(), img, params );
				}
  				//FeatEx().show(img);
			}

			bool addSep=false;
				if (!res.isEmpty())
				{
					
					if ( !rowIdType.isEmpty() )
					{
						if( addSep )
							*fs << ' ';
						*fs<<"\""<<fn.c_str()<<"\"";
						addSep=true;
					}
					if ( !classCol.isEmpty() )
					{
						if( addSep )
							*fs << ' ';
						*fs<<classCol;
						addSep=true;
					}

					foreach (double e, res)
					{
						if( addSep )
							*fs << ' ';

						*fs << e;
						addSep=true;
					}
					*fs<<"\n";
				}

#if 0
				
				//img = FeatEx().separateChannel(img, 1 );
				//FeatEx().show(img);

				QList<double> res;
				res += FeatEx().extractObjectFeatures( img, "basic" );
				
				img = FeatEx().op( "bilateralFilter", img, "3,6,6" );
				img = FeatEx().normalize(img, 255 );
				res += FeatEx().extractObjectFeatures( img, "haralick" );
				
				std::string imgtile = FeatEx().op( "tile", img, "0,0,2,2" );
				imgtile = FeatEx().normalize(imgtile, 255 );
				res += FeatEx().extractObjectFeatures( imgtile, "histogram" );

				imgtile = FeatEx().op( "tile", img, "0,1,2,2" );
				imgtile = FeatEx().normalize(imgtile, 255 );
				res += FeatEx().extractObjectFeatures( imgtile, "histogram" );

				imgtile = FeatEx().op( "tile", img, "1,0,2,2" );
				imgtile = FeatEx().normalize(imgtile, 255 );
				res += FeatEx().extractObjectFeatures( imgtile, "histogram" );

				imgtile = FeatEx().op( "tile", img, "1,1,2,2" );
				imgtile = FeatEx().normalize(imgtile, 255 );
				res += FeatEx().extractObjectFeatures( imgtile, "histogram" );

				imgtile = FeatEx().op( "tile", img, "1,1,3,3" );
				imgtile = FeatEx().normalize(imgtile, 255 );
				res += FeatEx().extractObjectFeatures( imgtile, "histogram" );
				

				res += FeatEx().extractObjectFeatures( img, "histogram" );
				

				imgtile = FeatEx().op( "tile", img, "0,0,2,2" );
				res += FeatEx().extractObjectFeatures( imgtile, "hog" );

				imgtile = FeatEx().op( "tile", img, "1,0,2,2" );
				res += FeatEx().extractObjectFeatures( imgtile, "hog" );

				imgtile = FeatEx().op( "tile", img, "0,1,2,2" );
				res += FeatEx().extractObjectFeatures( imgtile, "hog" );

				imgtile = FeatEx().op( "tile", img, "1,1,2,2" );
				res += FeatEx().extractObjectFeatures( imgtile, "hog" );


				//img = FeatEx().normalize(img, 255 );
				//res += FeatEx().extractObjectFeatures( img, "mat4x4" );
				//res += FeatEx().extractObjectFeatures( img, "mat16x16" );
				res += FeatEx().extractObjectFeatures( img, "mat8x8" );
				//res += FeatEx().extractObjectFeatures( img, "mat32x32" );

				//img = FeatEx().op( "sobel", img );
				//FeatEx().show(img);

				//std::string dft = FeatEx().op( "dftm", img );

				
				if ( verboseLevel > 0 )
				{
					qDebug() << fi.canonicalFilePath();
				}
				
#endif
		}
		catch(...){
		
		}

		if (cnt++ > limit) break;
	}
	delete fs;

	

	return 0;

}
