#include "haralick.h"

//#include <cv.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#include <iostream>
#include <fstream>
#include <cmath>



double Haralick::condAddSum(int n, cv::Mat & m)
{
  double s = 0.0;
  int w = m.rows;
  int h = m.cols;
  int size = w<h?w:h;

  for (int j=0; j<size; ++j)
  {
    if (2*j == n)
      s += m.at<unsigned char>(j,j);
    for (int i=j+1; i<size; ++i)
      if (i + j == n)
        s += 2 * m.at<unsigned char>(i,j);
  }
  return s;
}

void Haralick::clearGlcm( cv::Mat & m )
{
	//assert(m.rows() == m.cols());
	int n=m.rows;
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			m.at<float>(i,j)=0;
		}
	}
}

bool Haralick::printGlcm( cv::Mat & m, std::string fn )
{
	//assert(m.rows() == m.cols());
	int n=m.rows;

	std::ofstream f( fn.c_str() );

	for(int i=0; i<n; i++)
	{
		f <<"\n";
		for(int j=0; j<n; j++)
		{
			 f << m.at<float>(i,j)<<"\t";
		}
	}
	return f.good();
}


void Haralick::calcGlcm( cv::Mat m, int dist, cv::Mat &glcm, cv::Mat mask )
{
	int v=0;
	int dv90=0;
	int dv45=0;
	int dv135=0;
	int dv=0;
	int x=0;
	int y=0;

	
	cv::Mat glcm0 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm90 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm45 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm135 = cv::Mat(glcm.size(), CV_32FC1);

	clearGlcm(glcm);
	clearGlcm(glcm90);
	clearGlcm(glcm45);
	clearGlcm(glcm135);

	try{
		for (y=dist; y<m.cols-dist; ++y)
		{
			for (x=dist; x<m.rows-dist; ++x)
			{
				bool usePixel = true;
				if ( mask.rows > 0 && mask.cols > 0 )
				{
					usePixel &= mask.at<unsigned char>(x,y);
					usePixel &= mask.at<unsigned char>(x+dist,y); 
					usePixel &= mask.at<unsigned char>(x,y+dist); 
					usePixel &= mask.at<unsigned char>(x+dist,y+dist);
					usePixel &= mask.at<unsigned char>(x-dist,y-dist);
				}
				if (!usePixel) { 
					continue;
				}

				v = m.at<unsigned char>(x,y);
				dv = m.at<unsigned char>(x+dist,y);
				dv90 = m.at<unsigned char>(x,y+dist);
				dv45 = m.at<unsigned char>(x+dist,y+dist);
				dv135 = m.at<unsigned char>(x-dist,y-dist);

				glcm.at<float>(v,dv) = glcm.at<float>(v,dv) +1;
				glcm.at<float>(dv,v) = glcm.at<float>(v,dv);
				glcm90.at<float>(v,dv90) = glcm90.at<float>(v,dv90) +1;
				glcm90.at<float>(dv90,v) = glcm90.at<float>(v,dv90);
				glcm90.at<float>(v,dv45) = glcm45.at<float>(v,dv45) +1;
				glcm90.at<float>(dv45,v) = glcm45.at<float>(v,dv45);
				glcm90.at<float>(v,dv135) = glcm135.at<float>(v,dv135) +1;
				glcm90.at<float>(dv135,v) = glcm135.at<float>(v,dv135);
			}
		}
		cv::add( glcm,glcm90,glcm );
		cv::add( glcm,glcm45,glcm );
		cv::add( glcm,glcm135,glcm );
		glcm = glcm/4;

	}
	catch(...)
	{
	
	}
}

int Haralick::cntNz( cv::Mat & m  )
{
	int n= m.rows;
	int nz = 0;
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			nz +=m.at<float>(i,j);
		}
	}
	return nz;
}



double Haralick::condAddSum(cv::Mat & m, int n, int nmax)
{
  double s = 0.0;
  for (int j=0; j<nmax; ++j)
  {
    if (2*j == n)
      s += m.at<float>(j,j);
    for (int i=j+1; i<nmax; ++i)
      if (i + j == n)
        s += 2 * m.at<float>(i,j);
  }
  return s;
}

/**
* sum over all matrix elements where (i,j), where i-j == n
*/
double Haralick::condSubSum(cv::Mat & m, int n, int nmax)
{
  double s = (n == 0) ? m.at<float>(0,0) : 0.0;
  for (int j=0; j<nmax; ++j)
    for (int i=j+1; i<nmax; ++i)
		if (std::abs(i - j) == n)
        s += 2 * m.at<float>(i,j);
  return s;
}

void Haralick::normGlcm( cv::Mat & m, float num, int n  )
{
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
			m.at<float>(i,j) = m.at<float>(i,j) / num;
}

int Haralick::testglcm()
{
	cv::Mat m(6,6,CV_8UC1);
	m.at<unsigned char>(0,0)=0;m.at<unsigned char>(1,0)=0;m.at<unsigned char>(2,0)=0;m.at<unsigned char>(3,0)=0;m.at<unsigned char>(4,0)=0;m.at<unsigned char>(5,0)=0;
	m.at<unsigned char>(0,1)=0;m.at<unsigned char>(1,1)=1;m.at<unsigned char>(2,1)=4;m.at<unsigned char>(3,1)=3;m.at<unsigned char>(4,1)=4;m.at<unsigned char>(5,1)=0;
	m.at<unsigned char>(0,2)=0;m.at<unsigned char>(1,2)=2;m.at<unsigned char>(2,2)=2;m.at<unsigned char>(3,2)=3;m.at<unsigned char>(4,2)=4;m.at<unsigned char>(5,2)=0;
	m.at<unsigned char>(0,3)=0;m.at<unsigned char>(1,3)=3;m.at<unsigned char>(2,3)=1;m.at<unsigned char>(3,3)=2;m.at<unsigned char>(4,3)=3;m.at<unsigned char>(5,3)=0;
	m.at<unsigned char>(0,4)=0;m.at<unsigned char>(1,4)=4;m.at<unsigned char>(2,4)=2;m.at<unsigned char>(3,4)=4;m.at<unsigned char>(4,4)=4;m.at<unsigned char>(5,4)=0;
	m.at<unsigned char>(0,5)=0;m.at<unsigned char>(1,5)=0;m.at<unsigned char>(2,5)=0;m.at<unsigned char>(3,5)=0;m.at<unsigned char>(4,5)=0;m.at<unsigned char>(5,5)=0;
	const int MaxGl = 5;
	cv::Mat glcm(MaxGl,MaxGl,CV_32FC1);

	clearGlcm( glcm );

	calcGlcm( m, 1,glcm );
	cntNz( glcm );
 return 0;
}

cv::Mat Haralick::extractHaralicFeatures( cv::Mat img, int d, cv::Mat mask )
{
	Mat ret(1, 13, CV_32FC1);

	cv::Mat img_tmp = img.clone();

	const int MaxGl = 8;
	cv::Mat glcm(MaxGl,MaxGl,CV_32FC1);
	clearGlcm( glcm );

	cv::normalize(img_tmp, img_tmp,0,MaxGl-1, CV_MINMAX);

	if(0){
		Mat tmpview;
		cv::normalize(img_tmp, tmpview,0,255, CV_MINMAX);
		cv::imshow( "tmpview", tmpview );
		cv::waitKey(1);
	}

	calcGlcm( img_tmp, d, glcm, mask );
	int num = cntNz(glcm);
	normGlcm(glcm,num);
			

	double avg = cv::mean(img)[0];		  

	// calculate the mean (average)
	double average = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		average += j*glcm.at<float>(j,j);
		for (int i=j+1; i<MaxGl; ++i)
		  average += 2*j*glcm.at<float>(i,j);
	}

	// calculate the variance
	double variance = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		double var_i = glcm.at<float>(j,j);
		for (int i=j+1; i<MaxGl; ++i)
		  var_i += 2 * glcm.at<float>(i,j);

		variance += var_i * ((j - average)*(j - average));
	}
	
	double res = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		res += (glcm.at<float>(j,j))*(glcm.at<float>(j,j));
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * ((glcm.at<float>(j,i))*(glcm.at<float>(j,i)));
	}
	ret.at<float>(0,0)=res;
		
	/**
	* Inverse Difference Moment (Homogeneity}
	*/
    res = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		res += glcm.at<float>(j,j);
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * (glcm.at<float>(j,i) / (1 + ((i - j)*(i - j))));
	}
    ret.at<float>(0,1)=res;

	/**
     * Entropy
     */
    res = 0.0;
    double shift = 0.0001;
	for (int j=0; j<MaxGl; ++j)
	{
		res += glcm.at<float>(j,j) * std::log(glcm.at<float>(j,j) + shift);
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * glcm.at<float>(j,i) * log(glcm.at<float>(j,i) + shift);
	}
	ret.at<float>(0,2)=res;

	/**
     * Variance
     */
	res = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		res += glcm.at<float>(j,j) * ((j - average)*(j - average));
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * glcm.at<float>(i,j) * ((j - average)*(j - average));
	}
    ret.at<float>(0,3)=res;

	/**
     * Contrast (Interia)
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * glcm.at<float>(i,j) * ((i - j)*(i - j));
    ret.at<float>(0,4)=res;

    /**
     * Correlation
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += ((j - average)*(j - average)) * glcm.at<float>(j,j) / (variance*variance);
        for (int i=j+1; i<MaxGl; ++i)
          res += (2 * (i - average) * (j - average) *  glcm.at<float>(i,j) /
                  (variance*variance));
      }
    ret.at<float>(0,5)=res;
	/**
     * Prominence
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += pow((2 * j - 2 * average),4) * glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * std::pow((i + j - 2 * average),4) * glcm.at<float>(i,j);
      }
    ret.at<float>(0,6)=res;

    /**
     * Shade
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += pow((2 * j - 2 * average),3) * glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * std::pow((i + j - 2 * average),3) * glcm.at<float>(i,j);
      }
    ret.at<float>(0,7)=res;

    /**
     * Sum Average
     */
    res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);
    ret.at<float>(0,8)=res;

    /**
     * Sum Variance
     */
	res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);

	  double sav = res;
	  res = 0.0;
      
      for (int n=2; n <= 2*MaxGl; ++n)
        res += ((n - sav)*(n - sav)) * condAddSum(glcm,n,MaxGl);
    ret.at<float>(0,9)=res;

    /**
     * Sum Entropy
     */
	res = 0.0;
	for (int n=2; n <= 2*MaxGl; ++n)
	{
		res += n * condAddSum(glcm,n,MaxGl);
	}

	sav = res;
	res = 0.0;
	shift = 0.0001;
	for (int n=2; n <= 2*MaxGl; ++n)
	{
		double cas = condAddSum(glcm,n,MaxGl) + shift;
		res += cas * log(cas);
	}
    ret.at<float>(0,10)=res;

    /**
     * Difference Average
     */
	res = 0.0;
	for (int n=0; n < MaxGl; ++n)
	{
		res += n * condSubSum(glcm,n,MaxGl);
	}
    ret.at<float>(0,11)=res;

    /**
     * Coefficient of Variation
     */
    ret.at<float>(0,12)= variance / average;

	return ret;
}

#if 0
///OLD
#include "haralick.h"

//#include <cv.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

#include <iostream>
#include <fstream>
#include <cmath>



double Haralick::condAddSum(int n, cv::Mat & m)
{
  double s = 0.0;
  int w = m.rows;
  int h = m.cols;
  int size = w<h?w:h;

  for (int j=0; j<size; ++j)
  {
    if (2*j == n)
      s += m.at<unsigned char>(j,j);
    for (int i=j+1; i<size; ++i)
      if (i + j == n)
        s += 2 * m.at<unsigned char>(i,j);
  }
  return s;
}

void Haralick::clearGlcm( cv::Mat & m )
{
	//assert(m.rows() == m.cols());
	int n=m.rows;
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			m.at<float>(i,j)=0;
		}
	}
}

bool Haralick::printGlcm( cv::Mat & m, std::string fn )
{
	//assert(m.rows() == m.cols());
	int n=m.rows;

	std::ofstream f( fn.c_str() );

	for(int i=0; i<n; i++)
	{
		f <<"\n";
		for(int j=0; j<n; j++)
		{
			 f << m.at<float>(i,j)<<"\t";
		}
	}
	return f.good();
}


void Haralick::calcGlcm( cv::Mat m, int dist, cv::Mat &glcm )
{
	int v=0;
	int dv90=0;
	int dv45=0;
	int dv135=0;
	int dv=0;
	int x=0;
	int y=0;

	
	cv::Mat glcm0 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm90 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm45 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm135 = cv::Mat(glcm.size(), CV_32FC1);

	clearGlcm(glcm);
	clearGlcm(glcm90);
	clearGlcm(glcm45);
	clearGlcm(glcm135);

	try{
		for (y=dist; y<m.cols-dist; ++y)
		{
			for (x=dist; x<m.rows-dist; ++x)
			{
				v = m.at<unsigned char>(x,y);
				dv = m.at<unsigned char>(x+dist,y); //distant value
				dv90 = m.at<unsigned char>(x,y+dist); //distant value
				dv45 = m.at<unsigned char>(x+dist,y+dist); //distant value
				dv135 = m.at<unsigned char>(x-dist,y-dist); //distant value

				glcm.at<float>(v,dv) = glcm.at<float>(v,dv) +1;
				glcm.at<float>(dv,v) = glcm.at<float>(v,dv);
				glcm90.at<float>(v,dv90) = glcm90.at<float>(v,dv90) +1;
				glcm90.at<float>(dv90,v) = glcm90.at<float>(v,dv90);
				glcm90.at<float>(v,dv45) = glcm45.at<float>(v,dv45) +1;
				glcm90.at<float>(dv45,v) = glcm45.at<float>(v,dv45);
				glcm90.at<float>(v,dv135) = glcm135.at<float>(v,dv135) +1;
				glcm90.at<float>(dv135,v) = glcm135.at<float>(v,dv135);
			}
		}
		cv::add( glcm,glcm90,glcm );
		cv::add( glcm,glcm45,glcm );
		cv::add( glcm,glcm135,glcm );
		glcm = glcm/4;

	}
	catch(...)
	{
	
	}
}

int Haralick::cntNz( cv::Mat & m  )
{
	int n= m.rows;
	int nz = 0;
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			nz +=m.at<float>(i,j);
		}
	}
	return nz;
}



double Haralick::condAddSum(cv::Mat & m, int n, int nmax)
{
  double s = 0.0;
  for (int j=0; j<nmax; ++j)
  {
    if (2*j == n)
      s += m.at<float>(j,j);
    for (int i=j+1; i<nmax; ++i)
      if (i + j == n)
        s += 2 * m.at<float>(i,j);
  }
  return s;
}

/**
* sum over all matrix elements where (i,j), where i-j == n
*/
double Haralick::condSubSum(cv::Mat & m, int n, int nmax)
{
  double s = (n == 0) ? m.at<float>(0,0) : 0.0;
  for (int j=0; j<nmax; ++j)
    for (int i=j+1; i<nmax; ++i)
		if (std::abs(i - j) == n)
        s += 2 * m.at<float>(i,j);
  return s;
}

void Haralick::normGlcm( cv::Mat & m, float num, int n  )
{
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
			m.at<float>(i,j) = m.at<float>(i,j) / num;
}

int Haralick::testglcm()
{
	cv::Mat m(6,6,CV_8UC1);
	m.at<unsigned char>(0,0)=0;m.at<unsigned char>(1,0)=0;m.at<unsigned char>(2,0)=0;m.at<unsigned char>(3,0)=0;m.at<unsigned char>(4,0)=0;m.at<unsigned char>(5,0)=0;
	m.at<unsigned char>(0,1)=0;m.at<unsigned char>(1,1)=1;m.at<unsigned char>(2,1)=4;m.at<unsigned char>(3,1)=3;m.at<unsigned char>(4,1)=4;m.at<unsigned char>(5,1)=0;
	m.at<unsigned char>(0,2)=0;m.at<unsigned char>(1,2)=2;m.at<unsigned char>(2,2)=2;m.at<unsigned char>(3,2)=3;m.at<unsigned char>(4,2)=4;m.at<unsigned char>(5,2)=0;
	m.at<unsigned char>(0,3)=0;m.at<unsigned char>(1,3)=3;m.at<unsigned char>(2,3)=1;m.at<unsigned char>(3,3)=2;m.at<unsigned char>(4,3)=3;m.at<unsigned char>(5,3)=0;
	m.at<unsigned char>(0,4)=0;m.at<unsigned char>(1,4)=4;m.at<unsigned char>(2,4)=2;m.at<unsigned char>(3,4)=4;m.at<unsigned char>(4,4)=4;m.at<unsigned char>(5,4)=0;
	m.at<unsigned char>(0,5)=0;m.at<unsigned char>(1,5)=0;m.at<unsigned char>(2,5)=0;m.at<unsigned char>(3,5)=0;m.at<unsigned char>(4,5)=0;m.at<unsigned char>(5,5)=0;
	const int MaxGl = 5;
	cv::Mat glcm(MaxGl,MaxGl,CV_32FC1);

	clearGlcm( glcm );

	calcGlcm( m, 1,glcm );
	cntNz( glcm );
 return 0;
}

cv::Mat Haralick::extractHaralicFeatures( cv::Mat img, int d )
{
	Mat ret(1, 13, CV_32FC1);

	cv::Mat img_tmp = img.clone();

	const int MaxGl = 8;
	cv::Mat glcm(MaxGl,MaxGl,CV_32FC1);
	clearGlcm( glcm );

	cv::normalize(img_tmp, img_tmp,0,MaxGl-1, CV_MINMAX);
	calcGlcm( img_tmp, d, glcm );
	int num = cntNz(glcm);
	normGlcm(glcm,num);
			

	double avg = cv::mean(glcm)[0];		  

	// calculate the mean (average)
	double average = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		average += j*glcm.at<float>(j,j);
		for (int i=j+1; i<MaxGl; ++i)
		  average += 2*j*glcm.at<float>(i,j);
	}

	// calculate the variance
	double variance = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		double var_i = glcm.at<float>(j,j);
		for (int i=j+1; i<MaxGl; ++i)
		  var_i += 2 * glcm.at<float>(i,j);

		variance += var_i * ((j - average)*(j - average));
	}
	
	double res = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		res += (glcm.at<float>(j,j))*(glcm.at<float>(j,j));
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * ((glcm.at<float>(j,i))*(glcm.at<float>(j,i)));
	}
	ret.at<float>(0,0)=res;
		
	/**
	* Inverse Difference Moment (Homogeneity}
	*/
    res = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		res += glcm.at<float>(j,j);
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * (glcm.at<float>(j,i) / (1 + ((i - j)*(i - j))));
	}
    ret.at<float>(0,1)=res;

	/**
     * Entropy
     */
    res = 0.0;
    double shift = 0.0001;
	for (int j=0; j<MaxGl; ++j)
	{
		res += glcm.at<float>(j,j) * std::log(glcm.at<float>(j,j) + shift);
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * glcm.at<float>(j,i) * log(glcm.at<float>(j,i) + shift);
	}
	ret.at<float>(0,2)=res;

	/**
     * Variance
     */
	res = 0.0;
	for (int j=0; j<MaxGl; ++j)
	{
		res += glcm.at<float>(j,j) * ((j - average)*(j - average));
		for (int i=j+1; i<MaxGl; ++i)
		  res += 2 * glcm.at<float>(i,j) * ((j - average)*(j - average));
	}
    ret.at<float>(0,3)=res;

	/**
     * Contrast (Interia)
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * glcm.at<float>(i,j) * ((i - j)*(i - j));
    ret.at<float>(0,4)=res;

    /**
     * Correlation
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += ((j - average)*(j - average)) * glcm.at<float>(j,j) / (variance*variance);
        for (int i=j+1; i<MaxGl; ++i)
          res += (2 * (i - average) * (j - average) *  glcm.at<float>(i,j) /
                  (variance*variance));
      }
    ret.at<float>(0,5)=res;
	/**
     * Prominence
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += pow((2 * j - 2 * average),4) * glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * std::pow((i + j - 2 * average),4) * glcm.at<float>(i,j);
      }
    ret.at<float>(0,6)=res;

    /**
     * Shade
     */
    res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += pow((2 * j - 2 * average),3) * glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * std::pow((i + j - 2 * average),3) * glcm.at<float>(i,j);
      }
    ret.at<float>(0,7)=res;

    /**
     * Sum Average
     */
    res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);
    ret.at<float>(0,8)=res;

    /**
     * Sum Variance
     */
	res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);

	  double sav = res;
	  res = 0.0;
      
      for (int n=2; n <= 2*MaxGl; ++n)
        res += ((n - sav)*(n - sav)) * condAddSum(glcm,n,MaxGl);
    ret.at<float>(0,9)=res;

    /**
     * Sum Entropy
     */
	res = 0.0;
	for (int n=2; n <= 2*MaxGl; ++n)
	{
		res += n * condAddSum(glcm,n,MaxGl);
	}

	sav = res;
	res = 0.0;
	shift = 0.0001;
	for (int n=2; n <= 2*MaxGl; ++n)
	{
		double cas = condAddSum(glcm,n,MaxGl) + shift;
		res += cas * log(cas);
	}
    ret.at<float>(0,10)=res;

    /**
     * Difference Average
     */
	res = 0.0;
	for (int n=0; n < MaxGl; ++n)
	{
		res += n * condSubSum(glcm,n,MaxGl);
	}
    ret.at<float>(0,11)=res;

    /**
     * Coefficient of Variation
     */
    ret.at<float>(0,12)= variance / average;

	return ret;
}

#endif