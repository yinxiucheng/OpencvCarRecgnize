//
// Created by xiucheng yin on 2018/11/15.
//
#include "CarPlateLocation.h"
#include <vector>

using namespace std;


CarPlateLocation::CarPlateLocation() {

}


CarPlateLocation::~CarPlateLocation() {

}

void CarPlateLocation::location(Mat src, Mat &dst) {
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
    vector<Mat> plates;
    // 整个图片+经过初步赛选的车牌 + 得到的候选车牌
    tortuosity(src, vec_sobel_roi, plates);
    //更进一步的筛选
    //借助svm 进一步筛选
//    imshow("找到轮廓", src);


    blur.release();
    gray.release();
    close.release();

}

int CarPlateLocation::verifySizes(RotatedRect rotated_rect) {
    //容错率
    float error = 0.75f;

    //训练时候模型的宽高 136 * 32
    //获得宽高比
    float aspect = float(136) / float(32);

    //最小 最大面积 不符合的丢弃
    //给个大概就行 随时调整
    //尽量给大一些没关系， 这还是初步筛选。
    int min = 20 * aspect * 20;
    int max = 180 * aspect * 180;

    //比例浮动 error认为也满足
    //最小宽、高比
    float rmin = aspect - aspect * error;
    //最大的宽高比
    float rmax = aspect + aspect * error;
    //矩形的面积
    float area = rotated_rect.size.height * rotated_rect.size.width;
    //矩形的比例
    float r = (float) rotated_rect.size.width / (float) rotated_rect.size.height;
    if ((area < min || area > max) || (r < rmin || r > rmax))
        return 0;
    return 1;
}


/**
* 矫正
*/
void CarPlateLocation::tortuosity(Mat src, vector<RotatedRect> &rects, vector<Mat> &dst_plates) {
    //循环要处理的矩形
    for (RotatedRect roi_rect : rects) {
        float r = (float) roi_rect.size.width / (float) roi_rect.size.height;
        //矩形角度
        float roi_angle = roi_rect.angle;
        //矩形大小
        Size roi_rect_size = roi_rect.size;

        //让rect在一个安全的范围(不能超过src)
        Rect2f rect;
        safeRect(src, roi_rect, rect);


        //候选车牌
        //抠图  这里不是产生一张新图片 而是在src身上定位到一个Mat 让我们处理
        //数据和src是同一份
        Mat src_rect = src(rect);
        //真正的候选车牌
        Mat dst;
        //不需要旋转的 旋转角度小没必要旋转了
        if (roi_angle - 5 < 0 && roi_angle + 5 > 0) {
            dst = src_rect.clone();
        } else {
            //相对于roi的中心点 不减去左上角坐标是相对于整个图的
            //减去左上角则是相对于候选车牌的中心点 坐标
            Point2f roi_ref_center = roi_rect.center - rect.tl();
            Mat rotated_mat;
            //矫正 rotated_mat: 矫正后的图片
            rotation(src_rect, rotated_mat, roi_rect_size, roi_ref_center, roi_angle);
            dst = rotated_mat;
        }

        //定义大小
        Mat plate_mat;
        //高+宽
        plate_mat.create(32, 136, CV_8UC3);
        resize(dst, plate_mat, plate_mat.size());

        dst_plates.push_back(plate_mat);
        dst.release();
    }
}

//1、图片 2、要处理的矩形(可能超出有效范围) 3、处理之后在有效范围的矩形
void CarPlateLocation::safeRect(Mat src, RotatedRect rect, Rect2f &dst_rect) {

    //转为正常的带坐标的边框
    Rect2f boudRect = rect.boundingRect2f();
    //左上角 x,y
    float tl_x = boudRect.x > 0 ? boudRect.x : 0;
    float tl_y = boudRect.y > 0 ? boudRect.y : 0;
    //这里是拿 坐标 x，y 从0开始的 所以-1
    //右下角
    float br_x = boudRect.x + boudRect.width < src.cols
                 ? boudRect.x + boudRect.width - 1
                 : src.cols - 1;

    float br_y = boudRect.y + boudRect.height < src.rows
                 ? boudRect.y + boudRect.height - 1
                 : src.rows - 1;

    float w = br_x - tl_x;
    float h = br_y - tl_y;
    if (w <= 0 || h <= 0) return;
    dst_rect = Rect2f(tl_x, tl_y, w, h);
}


//1、矫正前 2、矫正后 3、矩形的大小 4、矩形中心点坐标  5、角度
void CarPlateLocation::rotation(Mat src, Mat &dst, Size rect_size,
                                Point2f center, double angle) {

    //获得旋转矩阵
    Mat rot_mat = getRotationMatrix2D(center, angle, 1);

    //运用仿射变换
    Mat mat_rotated;
    //矫正后 大小会不一样，但是对角线肯定能容纳
    int max = sqrt(pow(src.rows, 2) + pow(src.cols, 2));
    //仿射变换
    warpAffine(src, mat_rotated, rot_mat, Size(max, max),
               CV_INTER_CUBIC);
    imshow("旋转前", src);
//    imshow("旋转", mat_rotated);
    //截取 尽量把车牌多余的区域截取掉
    getRectSubPix(mat_rotated, Size(rect_size.width, rect_size.height), center, dst);
//    imshow("截取", dst);
    mat_rotated.release();
    rot_mat.release();
}




