#include "Convexity.h"
#include <QImage>
#include <pdimageprocessing.h>
#include <cells.h>
#include <vector>
#include <QtCore/qmath.h>
#include <QElapsedTimer>

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace cv;


QString ConvexityInfo ="Ratio of perimeters of the convex hull over the shape contour\n"
                       "type: 0-isConvex \n"
                       "metric: 0-mean, 1-variance";
REGISTER_FEATURE( Convexity, FeatureConvexity, ConvexityInfo );

FeatureConvexity::FeatureConvexity()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

	pi._name= "type";
	_paramInfos.append(pi);

    pi._name= "metric"; //0 - mean, 1 - variance
	_paramInfos.append(pi);

}

FeatureConvexity::~FeatureConvexity(){}

QString FeatureConvexity::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureConvexity::extract( QImage & img, TFeatParams params, QList<QRect> * rois )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int minNum=30;
	int metric = 0;
	int type = 0;


	QImage testImg;
	if (!params.contains("minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("maxCell"))
	{
		//todo error
		return res;
	}

	if (params.contains("metric"))
	{
		metric = params["metric"].toInt();
	}

	if (params.contains("type"))
	{
		type = params["type"].toInt();
	}


	QElapsedTimer timer;
	timer.start();

	int pyrFactor = 2; 

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(small));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();

	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	typedef std::vector<cv::Point> TContour;
	

	QString hk = small.text( "hashkey" );

	TRotatedRectList cells = ce.findCellRects( m );
	std::vector< TContour > conts = ce.cellContours();
	
	QList<double> values;

	Mat dbgimg = m.clone();   

	if(type == 5)
	{
		vector<Vec3f> circles;
		int i=0;
		foreach( TContour c,  conts )
			drawContours(dbgimg, conts, i++, Scalar(220), CV_FILLED);

		GaussianBlur( dbgimg, dbgimg, Size(3, 3), 2, 2 );
		//Canny( dbgimg, dbgimg, 100, 255, 3 );

		HoughCircles( dbgimg, circles, CV_HOUGH_GRADIENT, 1, 1, 220, 220, 0, 0 );

	  /// Draw the circles detected
	  /*for( size_t i = 0; i < circles.size(); i++ )
	  {
		  Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		  int radius = cvRound(circles[i][2]);
		  // circle center
		  circle( dbgimg, center, 3, Scalar(0,255,0), -1, 8, 0 );
		  //circle outline
		  circle( dbgimg, center, radius, Scalar(0,0,255), 3, 8, 0 );
	   }*/
	  values.append( circles.size() );
	}
	else
	{
		int i=0;
		foreach( TContour c,  conts )
		{
			//drawContours(dbgimg, conts, i++, Scalar(255), 0/*CV_FILLED*/);
			if (type == 0)
			{
				double isConv = cv::isContourConvex(c)?1.0:0.0;
				values.append( isConv );
			}
			if (type == 1) //convexity
			{
				double P = cv::arcLength(c,true);
				double A = cv::contourArea(c,false);
				//TContour ch;
				//cv::convexHull(c,ch);
				//double Ph = cv::arcLength(ch,true);
				values.append( (4*3.1415*A) / (P+P) );
			}
			if (type == 2) // ArcLen
			{
				double P = cv::arcLength(c,true);
				values.append( P );
			}
			if (type == 3) // Area		{
			{
				double A = cv::contourArea(c,true);
				values.append( A );
			}
			if(type == 4) // Area		
			{
				double A1 = cv::contourArea(c,true);
				TContour ch;
				cv::convexHull(c,ch);
				double A2 = cv::contourArea(ch,true);
				values.append( A1/A2 );
			}	
			if(type == 6)
			{
				std::vector<cv::Vec4i> convexityDefectsSet;   
					
				if (c.size() > 3)
				{
					//std::vector<int>  ch;
					//cv::convexHull(c,ch);
					TContour chp;
					cv::convexHull(c,chp);

					std::vector<TContour>  tmp;
					tmp.push_back(c);
					tmp.push_back(chp);
					drawContours(dbgimg, tmp, 0, Scalar(255), 0/*CV_FILLED*/);
					drawContours(dbgimg, tmp, 1, Scalar(0), 0/*CV_FILLED*/);

					//cv::convexityDefects( c, ch, convexityDefectsSet);
					//values.append ( convexityDefectsSet.size() );
					/*for( std::vector<cv::Vec4i>::const_iterator dit=convexityDefectsSet.begin(), dend=convexityDefectsSet.end(); dit!=dend; dit++ )
					{
						values.append( (*dit)[3] );

					}*/
				}
			}

			cv::Rect cr = boundingRect(c);
			positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
		}
	}

	//imshow("Convexity",dbgimg);

	QPair<double, double> result = PDImageProcessing::getMeanAndDev( values );

	if ( metric == 0 ) //mean
	{
		res->_value = result.first;
	}
	else if ( metric == 1 ) //variance
	{
		res->_value = result.second;
	}
	else
	{
		PD_ERROR("Undefine metric %d", metric);
	}


	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}