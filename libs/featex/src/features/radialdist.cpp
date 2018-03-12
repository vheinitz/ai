#include "radialdist.h"
#include <QImage>
#include <vector>
#include <QtCore/qmath.h>
#include <QElapsedTimer>

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

/*
TODO:

	RadialDistfactor      4 pi Area / Perimeter^2
Roundness       4 Area / (pi Max-diam^2)
Aspect ratio    Max-diam / Min-diam
Curl            max caliper diam / fiber length
Convexity       Convex Perimeter / Perimeter
Solidity        Area / Convex Area
Compactness     sqrt ( 4 * area / pi) / max diameter
Modification ratio    inscribed diameter / max diameter
Extent          Net Area / area of bounding rectangle


*/
using namespace cv;


QString RadialDistInfo ="Ratio of the area of a shape to the area of a circle having the same perimeter";
REGISTER_FEATURE( RadialDist, FeatureRadialDist, RadialDistInfo );

FeatureRadialDist::FeatureRadialDist()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

    pi._name= "metric"; //0 - mean, 1 - variance
	_paramInfos.append(pi);

}

FeatureRadialDist::~FeatureRadialDist(){}

QString FeatureRadialDist::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureRadialDist::extract( QImage & img, TFeatParams params, QList<QRect> * rois )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int minNum=30;
	int metric = 0;


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

	TRotatedRectList cells = ce.findCellRects( m,"",5 );
	std::vector< TContour > conts = ce.cellContours();



	QList<double> values;

	Mat debug = cv::Mat(small.height(), small.width(), CV_8UC1);
	int maxRowHight=0;
	int lastY = 0;
	int lastX = 0;

	for ( int ci=0; ci<ce.cells(); ci++ )
	{		
		Mat cell =  ce.cell(ci).clone() ;
		Mat cellDbgImg( 150, 100, CV_8UC1 );
		cellDbgImg=0;
		cell.copyTo( cellDbgImg(cv::Rect(0,0,cell.cols,cell.rows)) );
		cell = cellDbgImg(cv::Rect(0,0,cell.cols,cell.rows));

		double mean = cv::mean( cell )[0];
	
		Mat cellContour;
		int th = mean ;//*0.75 < 40 ? 40 : mean *0.75;
		
		cv::threshold(cell, cellContour, th, 255, CV_THRESH_BINARY ); 
		vector<vector<Point> > contours;
		findContours(cellContour, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);

		int maxIdx=-1;
		int nmax=0;

		

		cv::vector<int> convexHull;

		for (int i=0; i<contours.size();i++)
		{
			if ( contours.at(i).size() > nmax )
			{
				nmax = contours.at(i).size();
				maxIdx=i; 
			}			
		}

		char str[200];
		int textY=60; int tsize=10;

		if ( maxIdx>=0 && maxIdx< contours.size())
		{
			//DBG drawContours(cell, contours, maxIdx, Scalar(255), 0);

			Mat momentsShape( cell.size(), CV_8UC1 );
			momentsShape = 0;
			drawContours(momentsShape, contours, maxIdx, Scalar(255), CV_FILLED);

			int DefectCnt=0;
			int DefectMean=0;
			cv::convexHull(contours.at( maxIdx ), convexHull, true);
			
			textY=60;
			if ( convexHull.size() > 3 )
			{
				std::vector<cv::Vec4i> convexityDefectsSet;   
				
				cv::convexityDefects( contours.at( maxIdx ), convexHull, convexityDefectsSet);
				
				for( std::vector<cv::Vec4i>::const_iterator dit=convexityDefectsSet.begin(), dend=convexityDefectsSet.end(); dit!=dend; dit++ )
				{

					cv::Vec4i v = *dit;
					int defectSize = (v[3]/256);
					if ( defectSize>0 )
					{
						DefectCnt++;
						DefectMean+=defectSize;
						sprintf(str,"%d", defectSize);
						putText(cellDbgImg, str, Point2f(1,textY), FONT_HERSHEY_PLAIN, .5,  Scalar(255));
						textY+=tsize;
					}
				}
				if ( DefectCnt>0 )
				{
					DefectMean /= DefectCnt;
				}				

				if ( metric == 0 )
				{
					values<< DefectCnt;
				}
				else if ( metric == 1 )
				{
					values<< DefectMean;
				}
			}
		}		


		if ( maxRowHight <= cell.size().height ) 
		{
			maxRowHight = cellDbgImg.size().height;
		}

		//DBG  cellDbgImg.copyTo( debug( cv::Rect(lastX, lastY, cellDbgImg.size().width, cellDbgImg.size().height) ) );

		lastX += cellDbgImg.size().width + 5;
		if ( lastX >= debug.cols - cellDbgImg.size().width*2 )
		{
			lastX = 0;
			lastY += maxRowHight + 5;
			if ( lastY >= debug.rows-150 )
			{
				break;
			}
			maxRowHight=0;
		}
	
		///imshow("cell",cell);
		//cv::waitKey(1);
	}

	
	//DBG imshow("debug",debug);	
	

	qSort(values.begin(), values.end());
	QPair<double, double> result;
	
	if ( metric == 0 ) //Defect count
	{
		result = PDImageProcessing::getMeanAndDev( values.mid( 0, values.size()/2  ) );
	}
	else if ( metric == 1 ) //maximal defect 
	{
		result = PDImageProcessing::getMeanAndDev( values.mid( values.size()/2  ) );
	}
	

	//QPair<double, double> result = PDImageProcessing::getMeanAndDev( values );

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