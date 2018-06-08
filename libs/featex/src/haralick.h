#ifndef HARALICK_HG
#define HARALICK_HG
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Haralick
{
	cv::Mat _glcm;
private:
	void clearGlcm( cv::Mat & m );
	bool printGlcm( cv::Mat & m, std::string fn );
	double condAddSum(int n, cv::Mat & m);
    //OLD void calcGlcm( cv::Mat m, int dist, cv::Mat &glcm );
	void calcGlcm( cv::Mat m, int dist, cv::Mat &glcm, cv::Mat mask=cv::Mat() );
	int cntNz( cv::Mat & m  );
	double condAddSum(cv::Mat & m, int n, int nmax);
	double condSubSum(cv::Mat & m, int n, int nmax);
	void normGlcm( cv::Mat & m, float num, int n=8  );
	int testglcm();

public:
	Haralick( int maxGl=8 ):_glcm(maxGl,maxGl,CV_32FC1){} ;
    //OLD cv::Mat extractHaralicFeatures( cv::Mat img, int d );
	cv::Mat extractHaralicFeatures( cv::Mat img, int d, cv::Mat mask=cv::Mat() );
};

#endif