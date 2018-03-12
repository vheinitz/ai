#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <os/fstools.h>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
//#include <QUuid>
#include <QCryptographicHash>

#include <cells.h>

#include <fstream>


int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	bool append=false;
	bool colNames=false;
	QString classCol;
	QString rowIdType;
	int verboseLevel=0;
	int limit=999999999;
	cv::Size rs,outSize;
	bool cleanOutDir=false;
	QString outdir = "c:/tmp/objex/out";
	QString infile = "c:/tmp/objex/in";
	QString csv,lcsv,dcsv;
	bool csvOnly=false;
	int label=-1;
	
	foreach( QString arg, app.arguments() )
	{
		if (arg.contains("--infile=") )
		{
			infile = arg.section("=",1);
		}
		else if (arg.contains("--outdir=") )
		{
			outdir = arg.section("=",1);
		}
		else if (arg.contains("--csv=") )
		{
			csv = arg.section("=",1);
		}
		else if (arg.contains("--lcsv=") )
		{
			lcsv = arg.section("=",1);
		}
		else if (arg.contains("--dcsv=") )
		{
			dcsv = arg.section("=",1);
		}
		else if (arg.contains("--csvonly") )
		{
			csvOnly = true;;
		}
		else if (arg.contains("--label=") )
		{
			label = arg.section("=",1).toInt();
		}
		else if (arg.contains("--cleanoutdir") )
		{
			cleanOutDir=true;
		}
		else if (arg.contains("--rects") )
		{
			QString tmp = arg.section("=",1);
			rs.width = tmp.section("x",0,0).toInt();
			rs.height = tmp.section("x",1,1).toInt();
		}
		else if (arg.contains("--outsize") )
		{
			QString tmp = arg.section("=",1);
			outSize.width = tmp.section("x",0,0).toInt();
			outSize.height = tmp.section("x",1,1).toInt();
		}
		
	}

	if (!QFileInfo(infile).exists())
		return 2;

	if (!QFileInfo(outdir).exists())
		QDir().mkpath(outdir);

	if( cleanOutDir )
	{
		QDir d(outdir);
		QStringList tmpfiles = d.entryList(QStringList()<<"obex_*.png");

		foreach( QString f,  tmpfiles )
		{
			d.remove( outdir + "/" + f );
		}
	}	

	CellExtractor ce( 100, 300, 0, 6, 7 );

	QImage qimg( infile );
	QString hash = QCryptographicHash::hash( QByteArray( (const char *)qimg.bits(), qimg.byteCount()), QCryptographicHash::Md5).toHex();

	cv::Mat inp = _T::toMat( qimg );
	cv::Mat m;
	cv::normalize(inp,m,0,255,cv::NORM_MINMAX);
	//cv::imshow("fdfd", m);
	//cv::waitKey(5000);
	if( rs.area() == 0 )
	{
		TRotatedRectList rects = ce.findCellRects(m);
		for (int i=0; i< ce.cells(); i++)
		{		
			cv::Mat c = ce.cell(i);
			cv::Rect cr=ce.cellRect(i);
			QString fn = QString("obex_%1_%2_%3_%4x%5.png").arg(hash).arg(cr.x).arg(cr.y).arg(cr.width).arg(cr.height);
			
			if( !outSize.area() == 0 )
			{
				cv::resize( c, c ,cv::Size( outSize.width, outSize.height ) );
			}

			if( !csvOnly )
			{
				cv::imwrite(  QString("%1/%2").arg(outdir).arg(fn).toStdString(), c );
			}
			
			if( !csv.isEmpty() )
			{
				std::ofstream(csv.toStdString().c_str(),std::ofstream::out | std::ofstream::app)<<label<<","
				 << format(c, "CSV") << "\n";
			}

			if( !lcsv.isEmpty() )
			{
				std::ofstream(lcsv.toStdString().c_str(),std::ofstream::out | std::ofstream::app)<<label<< "\n";
			}

			if( !dcsv.isEmpty() )
			{
				std::ofstream(dcsv.toStdString().c_str(),std::ofstream::out | std::ofstream::app)<< format(c, "CSV") << "\n";
			}
		}
	}
	else
	{
		int xcnt= m.cols  / rs.width;
		int ycnt= m.rows  / rs.height;
		for (int x=0; x< xcnt-1; x++)
		{
			for (int y=0; y< ycnt-1; y++)
			{
				cv::Rect cr = cv::Rect(cv::Point(x*rs.width+50,y*rs.height+50),rs);
				cv::Mat c = m( cr );
				QString fn = QString("obex_%1_%2_%3_%4x%5.png").arg(hash).arg(cr.x).arg(cr.y).arg(cr.width).arg(cr.height);
				//cv::resize(c,c,cv::Size(32,32));
				//cv::imshow(" we" , c );
				//cv::waitKey(100);
				if( csv.isEmpty() )
					cv::imwrite(  QString("%1/%2").arg(outdir).arg(fn).toStdString(), c );
				else
				{
					std::ofstream(csv.toStdString().c_str(),std::ofstream::out | std::ofstream::app)<<label<<"," << format(c, "CSV") << "\n";;
				}
			}
		}
	}

	return 0;

}
