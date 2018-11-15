//
// Created by xiucheng yin on 2018/11/15.
//

#ifndef OPENCVCARRECGNIZE_CARCOLORPLATERECGNIZE_H
#define OPENCVCARRECGNIZE_CARCOLORPLATERECGNIZE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace cv;


class CarColorPlateLocation {
public:
    CarColorPlateLocation();
    ~CarColorPlateLocation();


    // 1、要定位的图片 2、引用类型 作为定位结果
    void location(Mat src, vector<Mat>& dst);

};


#endif //OPENCVCARRECGNIZE_CARCOLORPLATERECGNIZE_H
