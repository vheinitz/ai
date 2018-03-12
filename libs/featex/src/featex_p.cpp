#include "../include/featex.h"
#include "featex_p.h"
#include <QStringList>
#include <QDir>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QPainter>
#include <QtCore>
#include <QMutexLocker>
#include <QFileInfo>
#include <log/clog.h>
#include <QImage>
#include <QList>
#include <QRect>
#include <QMutexLocker>

#include "haralick.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "features/feature.h"


using namespace cv;
////////////////

#include <iostream>

using namespace cv;
using namespace std;

///////////
double medianMat(cv::Mat Input, int nVals){
    // COMPUTE HISTOGRAM OF SINGLE CHANNEL MATRIX
    float range[] = { 0, nVals };
    const float* histRange = { range };
    bool uniform = true; bool accumulate = false;
    cv::Mat hist;
    calcHist(&Input, 1, 0, cv::Mat(), hist, 1, &nVals, &histRange, uniform, accumulate);

    // COMPUTE CUMULATIVE DISTRIBUTION FUNCTION (CDF)
    cv::Mat cdf;
    hist.copyTo(cdf);
    for (int i = 1; i <= nVals-1; i++){
        cdf.at<float>(i) += cdf.at<float>(i - 1);
    }
    cdf /= Input.total();

     // COMPUTE MEDIAN
     double medianVal;
     for (int i = 0; i <= nVals-1; i++){
         if (cdf.at<float>(i) >= 0.5) { medianVal = i;  break; }
     }
     return medianVal/nVals; 
}
///////////


class ObjectDb
{
public:
	static ObjectDb & inst()
	{
		static ObjectDb inst;
		return inst;
	}

	std::map<std::string, cv::Mat> _imgdb;
};

FeatEx::FeatEx()
{
	_p  = QSharedPointer<FeatEx_p>( new FeatEx_p );
}


QList<double> FeatEx::extractObjectFeatures(  std::string obj, std::string featureSet,QVariant param  )
{
	return _p->extractObjectFeatures( obj, featureSet, param );
}

QStringList FeatEx::fnames( std::string featureSet, std::string prefix )
{
	return _p->fnames( featureSet, prefix );
}

void FeatEx::show(std::string img, bool pause)
{
	return _p->show( img,  pause );
}

std::string FeatEx::load( std::string fn, std::string obj )
{
	return _p->load(  fn,  obj );
}

std::string FeatEx::normalize( std::string src, int range, std::string dst )
{
	return _p->normalize(  src, range,  dst );
}

std::string FeatEx::crop( std::string src, int range, std::string dst )
{
	return _p->crop(  src, range,  dst );
}



std::string FeatEx::separateChannel( std::string src, int ch, std::string dst )
{
	return _p->separateChannel(  src,  ch,  dst );
}


std::string FeatEx::blur( std::string src, int dotsize, std::string dst )
{
	return _p->blur( src, dotsize, dst );
}

std::string FeatEx::op( std::string op, std::string src, std::string param, std::string dst )
{
	return _p->op( op, src, param, dst );
}


std::string FeatEx_p::load( std::string src, std::string dst )
{
	dst = dst.size()>0 ? dst: src;
	ObjectDb::inst()._imgdb[dst] = cv::imread( src,IMREAD_GRAYSCALE );
	return dst;
}

std::string FeatEx_p::normalize( std::string src, int range, std::string dst )
{
	dst = dst.size()>0 ? dst: src+"_N";
	cv::Mat tmp;
	int s = ObjectDb::inst()._imgdb.size();
	cv::normalize( ObjectDb::inst()._imgdb[src], tmp, 0, range, NORM_MINMAX, -1, Mat() );
	ObjectDb::inst()._imgdb[dst] = tmp;
	return dst;
}

std::string FeatEx_p::crop( std::string src, int range, std::string dst )
{
	dst = dst.size()>0 ? dst: src+"_N";
	cv::Mat tmp;
	int s = ObjectDb::inst()._imgdb.size();
	cv::normalize( ObjectDb::inst()._imgdb[src], tmp, 0, range, NORM_MINMAX, -1, Mat() );
	ObjectDb::inst()._imgdb[dst] = tmp;
	return dst;
}


std::string FeatEx_p::separateChannel( std::string src, int ch, std::string dst )
{
	dst = dst.size()>0 ? dst: src+"_C";
	Mat bgr[3];   
	split(ObjectDb::inst()._imgdb[src],bgr);
	ObjectDb::inst()._imgdb[dst] = bgr[ch];
	return dst;
}


std::string FeatEx_p::blur( std::string src, int dotsize, std::string dst )
{
	dst = dst.size()>0 ? dst: src+"_B";
	cv::Mat tmp;
	cv::GaussianBlur( ObjectDb::inst()._imgdb[src], tmp, cv::Size(dotsize, dotsize), 0);
	ObjectDb::inst()._imgdb[dst] = tmp;
	return dst;
}

std::string FeatEx_p::op( std::string op, std::string src, std::string param, std::string dst )
{
	dst = dst.size()>0 ? dst: src+op;
	if ( op == "bilateralFilter" )
	{
		cv::Mat tmp;
		cv::bilateralFilter( ObjectDb::inst()._imgdb[src], tmp, 3, 6, 6 );
		ObjectDb::inst()._imgdb[dst] = tmp;
	}
	else if ( op == "sobel" )
	{
		cv::Mat tmp;
   		cv::Sobel( ObjectDb::inst()._imgdb[src], tmp, CV_32F, 1, 0 );
		ObjectDb::inst()._imgdb[dst] = tmp;
	}
	else if ( op == "tile" )
	{
		QString tmp(param.c_str());
		cv::Mat m = ObjectDb::inst()._imgdb[src];
		int ridx = tmp.section(",",0,0).toInt(); // Index of tile
		int cidx = tmp.section(",",1,1).toInt(); // Index of tile
		int rows = tmp.section(",",2,2).toInt();
		int cols = tmp.section(",",3,3).toInt();
		cv::Rect roi( m.cols/cols*cidx, m.rows/rows*ridx, m.cols/cols, m.rows/rows);
		cv::Mat img = ObjectDb::inst()._imgdb[src];//
		cv::Mat tile = img(roi);//(roi).clone();
		ObjectDb::inst()._imgdb[dst] = tile;
	}
	else if ( op == "dftm" )
	{
		cv::Mat img = ObjectDb::inst()._imgdb[src];
		Mat padded;                            //expand input image to optimal size
		int m = getOptimalDFTSize( img.rows );
		int n = getOptimalDFTSize( img.cols ); // on the border add zero values
		copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols, BORDER_CONSTANT, Scalar::all(0));
		Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
		Mat complexI;
		merge(planes, 2, complexI); 

		cv::dft( complexI, complexI );
		split(complexI, planes);
		magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
		Mat magI = planes[0];

		magI += Scalar::all(1);                    // switch to logarithmic scale
		log(magI, magI);
		cv::normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
		ObjectDb::inst()._imgdb[dst] = magI;

		//cv::imshow("dftm", magI );
		//cv::imshow("img", img );
 		//cv::waitKey(0);
	}
	else if ( op == "devtest" ){
		cv::Mat img = ObjectDb::inst()._imgdb[src];
		img.convertTo(img, CV_32F, 1.0/255.0);
 
		// Calculate gradients gx, gy
		Mat gx, gy; 
		Sobel(img, gx, CV_32F, 1, 0, 1);
		Sobel(img, gy, CV_32F, 0, 1, 1);

		// C++ Calculate gradient magnitude and direction (in degrees)
		Mat mag, angle; 
		cartToPolar(gx, gy, mag, angle, 1); 
		cv::imshow("img", img );
		cv::imshow("mag", mag );
		cv::imshow("angle", angle );
 		cv::waitKey(0);
	}
	
	return dst;
}

QStringList FeatEx_p::fnames( std::string featureSet, std::string pfx )
{
	QStringList names;
	QString prefix = pfx.c_str();
	if ( featureSet == "basic")
	{
		names << prefix+"b_W" << prefix+"b_H" << prefix+"b_Min" << prefix+"b_Max" << prefix+"b_Avg" 
			<< prefix+"b_Var" << prefix+"b_Dev" << prefix+"b_Sum" << prefix+"b_SqSum" ; 
	}
	if ( featureSet == "haralick")
	{
		for (int i=0; i< 13; i++)
		{		
			names << QString("%2h_F%1").arg(i+1).arg(prefix ); 
		}
	}
	if ( featureSet == "histogram")
	{
		for (int i=0; i< 32; i++)
		{		
			names << QString("%2hg_B%1").arg(i+1).arg(prefix ); 
		}
	}
	if ( featureSet == "mat4x4")
	{
		for (int i=0; i< 4*4; i++)
		{		
			names << QString("%2m4_%1").arg(i+1).arg(prefix ); 
		}
	}
	if ( featureSet == "mat8x8")
	{
		for (int i=0; i< 8*8; i++)
		{		
			names << QString("%2m8_%1").arg(i+1).arg(prefix ); 
		}
	}
	if ( featureSet == "mat16x16")
	{
		for (int i=0; i< 16*16; i++)
		{		
			names << QString("%2m16_%1").arg(i+1).arg(prefix ); 
		}
	}
	if ( featureSet == "mat32x32")
	{
		for (int i=0; i< 32*32; i++)
		{		
			names << QString("%2m32_%1").arg(i+1).arg(prefix ); 
		}
	}
	if ( featureSet == "hog")
	{
		for (int i=0; i< 9; i++)
		{		
			names << QString("%2hog_%1").arg(i+1).arg(prefix ); 
		}
	}
	return names;
}

void FeatEx_p::show( std::string img, bool pause )
{
	//TODO check exist      
	cv::imshow( "Image", ObjectDb::inst()._imgdb[img] );
	cv::waitKey(pause?0:1);
}


cv::FastFeatureDetector fast_detector;
cv::GoodFeaturesToTrackDetector gft_detector;
cv::MserFeatureDetector mser_detector;


QList<double> extractBasicFeatures( std::string objid );

QMutex mx;
QList<double> FeatEx_p::extractObjectFeatures( std::string objid, std::string featureSet, QVariant param )
{	

	if (featureSet == "basic")
	{
		return extractBasicFeatures(  objid  );
	}

	if (featureSet == "haralick")
	{
		return extractHaralickFeatures(  objid  );
	}

	if (featureSet == "histogram")
	{
		return extractHistogramFeatures(  objid  );
	}
	if (featureSet == "mat4x4")
	{
		return extractMat(  objid , 4 );
	}
	if (featureSet == "mat8x8")
	{
		return extractMat(  objid , 8 );
	}
	if (featureSet == "mat16x16")
	{
		return extractMat(  objid , 16 );
	}
	if (featureSet == "mat32x32")
	{
		return extractMat(  objid , 32 );
	}
	if (featureSet == "hog")
	{
		return extractHOG(  objid  );
	}

	return QList<double>();

}




FeatEx_p::FeatEx_p(QObject *parent) :
    QObject(parent),
	_futureCnt(0),
	_stop(0)
{
	//PD_LOG("Feature extructor created" )
}

FeatEx_p::~FeatEx_p()
{	
	//PD_LOG("Feature extructor shut down" )
	_runningFuturesMx.lock();
	foreach( QFuture<double> f, _runningFutures )
	{
		f.cancel();
	}
	_runningFuturesMx.unlock();
	//_deleteMx.lock(); // make sure no other thread is using my methods
	//_deleteMx.unlock();
}


QList<double> FeatEx_p::extractBasicFeatures( std::string objid  )
{	
	QList<double> values;
	Mat roi = ObjectDb::inst()._imgdb[objid];	
	double min, max;
	cv::minMaxLoc (roi,&min,&max,0,0);
	int avg = cv::mean( roi )[0];
	cv::Mat     mean;
	cv::Mat     stddev;
	cv::meanStdDev ( roi, mean, stddev );
	Mat sqrsum = roi.mul(roi);

	values <<  roi.cols;
	values <<  roi.rows;
	values <<  min;
	values <<  max;
	values <<  avg;
	values << qSqrt(stddev.data[0]);
	values << stddev.data[0];
	values <<  cv::sum(roi)[0];
	values <<  cv::sum(sqrsum)[0];

	return values;
}

QList<double> FeatEx_p::extractHaralickFeatures( std::string objid  )
{	
	QList<double> values;
	Mat roi = ObjectDb::inst()._imgdb[objid];	
	Haralick h;
	Mat hf = h.extractHaralicFeatures( roi, 3 );

	for (int i=0; i< 13; i++)
	{		
		values << hf.at<float>(0,i);
	}

	return values;
}

QList<double> FeatEx_p::extractHistogramFeatures( std::string objid  )
{	
	QList<double> values;
	Mat roi = ObjectDb::inst()._imgdb[objid];	
	int histSize = 32;
	float range[] = { 0, 256 } ;
	const float* histRange = { range };
	Mat hist;
	calcHist( &roi, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false );
	for (int i=0; i< histSize; i++)
	{		
		values << hist.at<float>(i);
	}

	return values;
}

QList<double> FeatEx_p::extractMat( std::string objid, int size  )
{
	
	QList<double> values;
	Mat img = ObjectDb::inst()._imgdb[objid];	
	Mat m;
	cv::resize(img, m, Size(size, size));
	
	for (int r=0; r< size; r++)
	{		
		for (int c=0; c< size; c++)
			values << m.at<unsigned char>(c,r);
	}

	Mat ms;
	cv::resize(m, ms, Size(50, 50)); 
	//cv::imshow("img", img) ;
	//cv::imshow("small", ms) ;
	//cv::waitKey(0);

 	return values;
}

QList<double> FeatEx_p::extractHOG(  std::string objid  )
{
	
	QList<double> values;
	cv::Mat img = ObjectDb::inst()._imgdb[objid];	

	img.convertTo(img, CV_32F, 1.0/255.0);

	// Calculate gradients gx, gy
	Mat gx, gy; 
	Sobel(img, gx, CV_32F, 1, 0, 1);
	Sobel(img, gy, CV_32F, 0, 1, 1);
	QMap<int, float> tmp;
	QMap<int, float> hist;

	// C++ Calculate gradient magnitude and direction (in degrees)
	Mat mag, angle; 
	cartToPolar(gx, gy, mag, angle, 1); 
	
	for (int r=0; r< std::min(mag.rows, angle.rows); r++)
	{		
		for (int c=0; c< std::min(mag.cols, angle.cols); c++)
		{
			float m = mag.at<float>(r,c);
			float a = angle.at<float>(r,c);
			//int ang = a*180;
			tmp[a] += m;
		}
	}

	for (int i=0; i< 9; i++)hist[i]=0;

	foreach(int i, tmp.keys())
	{
		if( i>=180 ) break;
		hist[i/20] += tmp[i];
	}

	foreach(int i, tmp.keys())
	{
		if( i>=180 )
			hist[(i-180)/20] += tmp[i];
	}

	for (int i=0; i< 9; i++)values <<hist[i];

	return values;
}

#if 0

QList<double> FeatEx_p::extractBasicFeatures( std::string objid  )
{	
	QList<double> values;
	Mat cell = ObjectDb::inst()._imgdb[objid];
	
	Mat frameMask = Mat::zeros( cell.size(), CV_8UC1 );
	cv::rectangle(frameMask,cv::Point(0,0),cv::Point(cell.cols,cell.rows),cv::Scalar(255),1);	
	Mat sellBaseEnclosingMask = Mat::zeros( cell.size(), CV_8UC1 ); //Mask with no defects
	
	//imshow( "cent", cell );
	//cv::waitKey(1);
	
	int th_mean = cv::mean( cell )[0];

	////// NEW
	double min, max;
	cv::minMaxLoc (cell,&min,&max,0,0);
	int avg = cv::mean( cell )[0];
	

	float AR = float(std::min( cell.cols, cell.rows )) / std::max( cell.cols, cell.rows );

	values << cell.cols;
	if (fnames(  ).contains("RoiY") ) values["RoiY"] = cell.rows;
	if (fnames(  ).contains("Min") ) values["Min"] = min;
	if (fnames(  ).contains("Max") ) values["Max"] = max;
	if (fnames(  ).contains("Avg") ) values["Avg"] = avg;
	
	
	{ //StdDev, Var
		cv::Mat     mean;
		cv::Mat     stddev;
		cv::meanStdDev ( cell, mean, stddev );
		if (fnames(  ).contains("Var") ) values["Var"] = qSqrt(stddev.data[0]);
		if (fnames(  ).contains("StdDev") ) values["StdDev"] = stddev.data[0];
	}

	{//Sum, SqrSum
		if (fnames(  ).contains("Sum") ) values["Sum"] = cv::sum(cell)[0];
		Mat sqrsum = cell.mul(cell);
		if (fnames(  ).contains("SqrSum") ) values["SqrSum"] = cv::sum(sqrsum)[0];
	}

	{//Haralick
		Haralick h;
		Mat hf = h.extractHaralicFeatures( cell( cv::Rect( cv::Point( cell.cols/4, cell.rows/4 ),cv::Size( cell.cols/2, cell.rows/2 ) ) ), 3 );
		//Mat hf = h.extractHaralicFeatures( cell, 3 );

		for (int i=0; i< 13; i++)
		{		
			if (fnames(  ).contains( QString("HF%1").arg(i) ) ) values[QString("HF%1").arg(i)] = hf.at<float>(0,i);
		}
	}
	
	{ // Histogram
		int histSize = 16;
		float range[] = { 0, 256 } ;
		const float* histRange = { range };
		Mat hist;
		calcHist( &cell, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false );
		for (int i=0; i< 16; i++)
		{		
			if (fnames(  ).contains( QString("H%1").arg(i) ) ) values[QString("H%1").arg(i)] = hist.at<float>(i);
		}
	}

	if (fnames(  ).contains("FastN")) 
	{
		
		std::vector<cv::KeyPoint> keypoints_1;
		fast_detector.detect( cell, keypoints_1 );
		
		Mat m= cell.clone();
		double kpavg=0.0;
		double mimadiffavg=0.0;
		double kpresponseavg=0.0;
		int peaks=0;
		for(int i=0; i< keypoints_1.size(); i++)
		{
			//cv::circle(m, keypoints_1.at(i).pt,2,cv::Scalar(255));
			try
			{
				cv::Mat r = cell(cv::Rect(keypoints_1.at(i).pt.x-4, keypoints_1.at(i).pt.y-4, 8,8));
				double rm= cv::mean( r )[0];
				kpavg += rm;

				double min, max;
				cv::minMaxLoc (r,&min,&max,0,0);
				mimadiffavg += max-min;
				kpresponseavg += keypoints_1.at(i).response;
				if ( avg < rm*.75)
				{
					peaks++;
				}
			}
			catch(...){}
		}
		kpavg /=keypoints_1.size();
		//imshow("m",m);
		//cv::waitKey(1);
		values["FastN"]= keypoints_1.size();
 		values["FastKpAvg"]= kpavg/avg;
		values["FastKpAvg"]= kpavg/avg;
		values["FastMMDA"]= mimadiffavg/keypoints_1.size();
		values["FastKPRA"]= kpresponseavg/keypoints_1.size();
		values["Peaks"]= peaks;


	}

	return values.values();

	//END nw
}
#endif

///// OLD
int dft_test( const char * argc)
{
   

    const char* filename = argc;

    Mat I = imread(filename, IMREAD_GRAYSCALE);
	GaussianBlur( I, I, Size(5,5), 0, 0, BORDER_DEFAULT );
	//cv::pyrDown(I, I);
    if( I.empty()){
        cout << "Error opening image" << endl;
        return -1;
    }

//! [expand]
    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize( I.rows );
    int n = getOptimalDFTSize( I.cols ); // on the border add zero values
    copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));
//! [expand]

//! [complex_and_real]
    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);         // Add to the expanded another plane with zeros
//! [complex_and_real]

//! [dft]
    dft(complexI, complexI);            // this way the result may fit in the source matrix
//! [dft]

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
//! [magnitude]
    split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    Mat magI = planes[0];
//! [magnitude]

//! [log]
    magI += Scalar::all(1);                    // switch to logarithmic scale
    log(magI, magI);
//! [log]

//! [crop_rearrange]
    // crop the spectrum, if it has an odd number of rows or columns
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    int cx = magI.cols/2;
    int cy = magI.rows/2;

    Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
    Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
    Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

    Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);
//! [crop_rearrange]

//! [normalize]
	//circle(magI, Point(magI.cols/2,magI.rows/2), 20, Scalar(0),CV_FILLED, 8,0);
    normalize(magI, magI, 0, 1, NORM_MINMAX); // Transform the matrix with float values into a
                                            // viewable image form (float between values 0 and 1).
//! [normalize]

    //imshow("Input Image"       , I   );    // Show the result
    //imshow("spectrum magnitude", magI);	

	//normalize(magI, magI, 0, 1, NORM_MINMAX); // Transform the matrix with float values into a

	Mat th;
	cv::threshold(magI,th,.5, 255,cv::THRESH_BINARY);
	//imshow("TH", th);	

	cv::Mat inverseTransform;
    cv::dft(complexI, inverseTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);
    normalize(inverseTransform, inverseTransform, 0, 1, CV_MINMAX);
    //imshow("Reconstructed", inverseTransform);

	
 
  cv::waitKey(3000);

return 0;
}
////////////////

/*
TODO:

Formfactor      4 pi Area / Perimeter^2
Roundness       4 Area / (pi Max-diam^2)
Aspect ratio    Max-diam / Min-diam
Curl            max caliper diam / fiber length
Convexity       Convex Perimeter / Perimeter
Solidity        Area / Convex Area
Compactness     sqrt ( 4 * area / pi) / max diameter
Modification ratio    inscribed diameter / max diameter
Extent          Net Area / area of bounding rectangle


*/

cv::Size dbgImgSize( 1024, 768 );
int border = 2;
Mat dbgImg = Mat::zeros( dbgImgSize, CV_8UC1 );
int initImg(){ dbgImg = 60; return 0; }
static int initimg=initImg();

int roiX=0,roiY=0, roiRowMax=0;
int imgCnt=1;



void addTileToDbgImage( Mat & tile )
{
	if ( roiY+tile.rows >= dbgImgSize.height-border )
	{
		//imshow( "DEbugImage", dbgImg );
		QString imgName = QString("c:/temp/masks/mask_%1.png").arg(imgCnt++,5,10,QChar('0'));
		imwrite( imgName.toStdString(), dbgImg );
		cv::waitKey(1);
		//cv::waitKey(0);
 		dbgImg = 60;
		roiX=border;
		roiY=border;
		roiRowMax=0;
	}	
	
	if ( roiX+tile.cols >= dbgImgSize.width-border )
	{
		roiY+=border+roiRowMax;
		roiX=border;
		
	}
	tile.copyTo( dbgImg(cv::Rect(cv::Point(roiX,roiY),tile.size()) ) );
	roiX+=border+tile.cols;
	roiRowMax = std::max(roiRowMax, tile.rows );

}


double geometric_mean( Mat data) //??? 
{
    double m = 1.0;
    long long ex = 0;
    double invN = 1.0 / data.cols *data.rows;
	//unsigned char p;

    for (int y=0; y<data.cols; ++y)
	{
		for (int x=0; x<data.rows; ++x)
		{
			double p = data.at<unsigned char>(x,y);
			int i;
			double f1 = std::frexp(p,&i);
			m*=f1;
			ex+=i;
		}
    }

    return std::pow( std::numeric_limits<double>::radix,ex * invN) * std::pow(m,invN);
}

