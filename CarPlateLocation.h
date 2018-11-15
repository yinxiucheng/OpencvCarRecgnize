//
// Created by xiucheng yin on 2018/11/15.
//

#ifndef OPENCVCARRECGNIZE_CARPLATELOCATION_H
#define OPENCVCARRECGNIZE_CARPLATELOCATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace cv;

class CarPlateLocation {
public:
    CarPlateLocation();
    virtual ~CarPlateLocation();//父类的虚函数

protected:
    int verifySizes(RotatedRect rect);

    void tortuosity(Mat src, vector<RotatedRect> &rects, vector<Mat> &dst_plates);

    void safeRect(Mat src, RotatedRect rect, Rect2f &dst_rect);

    void rotation(Mat src, Mat &dst, Size rect_size, Point2f center, double angle);
};


#endif //OPENCVCARRECGNIZE_CARPLATELOCATION_H
