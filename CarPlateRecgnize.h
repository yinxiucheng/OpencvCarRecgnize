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
    CarPlateRecgnize(const char* svm_model,const char* ann_ch_path, const char* ann_path);
    ~CarPlateRecgnize();

    string plateRecgnize(Mat src);
private:
    CarSobelPlateLocation *sobelPlateLocation = 0;
    CarColorPlateLocation *colorPlateLocation = 0;
    Ptr<SVM> svm;
    HOGDescriptor *svmHog = 0;

    HOGDescriptor *annHog = 0;
    Ptr<ANN_MLP> annCh;
    Ptr<ANN_MLP> ann;
    static string ZHCHARS[];
    static char CHARS[];


    void getHogFeatures(HOGDescriptor *hog, Mat src, Mat &out);
    void clearFixPoint(Mat &src);
    bool verityCharSize(Mat mat);
    int getCityIndex(vector<Rect> vector);
    void getChineseRect(Rect city, Rect& chineseRect);
    void predict(vector<Mat> vector, String &str);
};


#endif //OPENCVCARRECGNIZE_CARPLATERECGNIZE_H
