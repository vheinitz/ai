#ifndef FEATURE_RadialDist_HG
#define FEATURE_RadialDist_HG

#include "feature.h"

//Implementation of the Feature RadialDist
//Various form features

class FeatureRadialDist : public Feature
{
public:
	FeatureRadialDist();
	virtual ~FeatureRadialDist();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

#endif