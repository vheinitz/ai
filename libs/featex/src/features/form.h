#ifndef FEATURE_Form_HG
#define FEATURE_Form_HG

#include "feature.h"

//Implementation of the Feature Form
//Various form features

class FeatureForm : public Feature
{
public:
	FeatureForm();
	virtual ~FeatureForm();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params, QList<QRect> * rois=0 );
};

#endif