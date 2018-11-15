//
// Created by xiucheng yin on 2018/11/15.
//
#include "CarSobelPlateLocation.h"
#include <vector>

using namespace std;


CarSobelPlateLocation::CarSobelPlateLocation() {

}

CarSobelPlateLocation::~CarSobelPlateLocation() {

}

void CarSobelPlateLocation::location(Mat src, vector<Mat> &dst) {
    //预处理 ：去噪 让车牌区域更加突出
    Mat blur;
    //1、高斯模糊（平滑） （1、为了后续操作 2、降噪 ）
    GaussianBlur(src, blur, Size(5, 5), 0);
    //imshow("高斯模糊",blur);

    Mat gray;
    //2、灰度化 去掉颜色 因为它对于我们这里没用  降噪
    cvtColor(blur, gray, COLOR_BGR2GRAY);
    imshow("灰度", gray);

    Mat sobel_16;
    //3、 边缘检测 让车牌更加突出  在调用时需要以16位来保存数据 在后续操作 以及显示的时候需要转回8位
    Sobel(gray, sobel_16, CV_16S, 1, 0);
    //转为8位
    Mat sobel;
    convertScaleAbs(sobel_16, sobel);
    //imshow("Sobel", sobel);

    //4. 二值化 黑白
    Mat shold;
    //大律法   最大类间算法
    threshold(sobel, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
    imshow("二值", shold);

    //5、闭操作
    // 将相邻的白色区域扩大 连接成一个整体
    Mat close;
    Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));
    morphologyEx(shold, close, MORPH_CLOSE, element);
    imshow("闭操作", close);

    //6、查找轮廓
    //获得初步筛选车牌轮廓================================================================
    //轮廓检测
    vector<vector<Point>> contours;
    //查找轮廓 提取最外层的轮廓  将结果变成点序列放入 集合
    findContours(close, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    //遍历
    vector<RotatedRect> vec_sobel_roi;
    for (vector<Point> point:contours) {
        RotatedRect rotatedRect = minAreaRect(point);
        //rectangle(src, rotatedRect.boundingRect(), Scalar(255, 0, 255));
        //进行初步的筛选 把完全不符合的轮廓给排除掉 ( 比如：1x1，5x1000 )
        if (verifySizes(rotatedRect)) {
            vec_sobel_roi.push_back(rotatedRect);
        }
    }

//    for (RotatedRect r : vec_sobel_roi) {
//        rectangle(src, r.boundingRect(), Scalar(255, 0, 255));
//    }
//    imshow("q1", src);

    //矫正
    //因为可能斜的，处理扭曲
    //获得候选车牌
//    vector<Mat> plates;
    // 整个图片+经过初步赛选的车牌 + 得到的候选车牌
    tortuosity(src, vec_sobel_roi, dst);

    blur.release();
    gray.release();
    close.release();
}






