//
// Created by xiucheng yin on 2018/11/15.
//

#ifndef OPENCVCARRECGNIZE_CARPLATERECGNIZE_H
#define OPENCVCARRECGNIZE_CARPLATERECGNIZE_H

#include "CarSobelPlateLocation.h"
#include "CarColorPlateLocation.h"
#include <string>
#include <opencv2/ml.hpp>


using namespace std;
using namespace ml;


class CarPlateRecgnize {
public:
    CarPlateRecgnize(const char* svm_model);
    ~CarPlateRecgnize();

    string plateRecgnize(Mat src);
private:
    CarSobelPlateLocation *sobelPlateLocation = 0;
    CarColorPlateLocation *colorPlateLocation = 0;
    Ptr<SVM> svm;
    HOGDescriptor *svmHog = 0;

    void getHogFeatures(HOGDescriptor *hog, Mat src, Mat &out);
};


#endif //OPENCVCARRECGNIZE_CARPLATERECGNIZE_H
