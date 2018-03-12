#ifndef TESTING_HG
#define TESTING_HG

#include "feature.h"

//TODO Implement Features:
// http://www.math.uci.edu/icamp/summer/research_11/park/shape_descriptors_survey.pdf
// circularity
// Eccentricity
// Rectangularity
// Convexity
// Solidity: Shape / Convex Hull
// Euler Number: Connected Regions - Holes
// Hole Area Ratio: Area of Shape / Area of holes
// Mingqiang Yang, Kidiyo Kpalma, Joseph Ronsin. A Survey of Shape Feature Extraction Techniques. Peng-Yeng Yin. Pattern Recognition, IN-TECH, pp.43-90, 2008. <hal-00446037>


class FeatureSpecialANCA : public Feature
{
public:
	FeatureSpecialANCA();
	virtual ~FeatureSpecialANCA();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureLocalMax : public Feature
{
public:
	FeatureLocalMax();
	virtual ~FeatureLocalMax();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeaturePeaks : public Feature
{
public:
	FeaturePeaks();
	virtual ~FeaturePeaks();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureCellPeaks : public Feature
{
public:
	FeatureCellPeaks();
	virtual ~FeatureCellPeaks();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureLayersRatio : public Feature
{
public:
	FeatureLayersRatio();
	virtual ~FeatureLayersRatio();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureMembraneMeanRatio : public Feature
{
public:
	FeatureMembraneMeanRatio();
	virtual ~FeatureMembraneMeanRatio();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeaturePeaksPerLine : public Feature
{
public:
	FeaturePeaksPerLine();
	virtual ~FeaturePeaksPerLine();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureEllipses : public Feature
{
public:
	FeatureEllipses();
	virtual ~FeatureEllipses();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};


class FeatureBlobs : public Feature
{
public:
	FeatureBlobs();
	virtual ~FeatureBlobs();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureNdnaIndex : public Feature
{
public:
	FeatureNdnaIndex();
	virtual ~FeatureNdnaIndex();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params , QList<QRect> * rois=0);
};

//TODO
class FeatureExposure : public Feature
{
public:
	FeatureExposure();
	virtual ~FeatureExposure();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureImgParam : public Feature
{
public:
	FeatureImgParam();
	virtual ~FeatureImgParam();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureNumOfStroks : public Feature
{
public:
	FeatureNumOfStroks();
	virtual ~FeatureNumOfStroks();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureSpinsPerLine : public Feature
{
public:
	FeatureSpinsPerLine();
	virtual ~FeatureSpinsPerLine();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureMatchHistogram : public Feature
{
public:
	FeatureMatchHistogram();
	virtual ~FeatureMatchHistogram();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureSequence : public Feature
{
public:
	FeatureSequence();
	virtual ~FeatureSequence();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureHuMean : public Feature
{
public:
	FeatureHuMean();
	virtual ~FeatureHuMean();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureCellMetrics : public Feature
{
public:
	FeatureCellMetrics();
	virtual ~FeatureCellMetrics();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureExif : public Feature
{
public:
	FeatureExif();
	virtual ~FeatureExif();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureCellVariance : public Feature
{
public:
	FeatureCellVariance();
	virtual ~FeatureCellVariance();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureCellTexture : public Feature
{
public:
	FeatureCellTexture();
	virtual ~FeatureCellTexture();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeaturePosMitosis : public Feature
{
public:
	FeaturePosMitosis();
	virtual ~FeaturePosMitosis();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureCellShape : public Feature
{
public:
	FeatureCellShape();
	virtual ~FeatureCellShape();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeatureFindTempl : public Feature
{
public:
	FeatureFindTempl();
	virtual ~FeatureFindTempl();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeaturePosNDna : public Feature
{
public:
	FeaturePosNDna();
	virtual ~FeaturePosNDna();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

class FeaturePeakParams : public Feature
{
public:
	FeaturePeakParams();
	virtual ~FeaturePeakParams();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

#endif