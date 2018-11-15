//
// Created by xiucheng yin on 2018/11/15.
//

#include "CarPlateRecgnize.h"

CarPlateRecgnize::CarPlateRecgnize(const char* svm_model) {
    sobelPlateLocation = new CarSobelPlateLocation();
    colorPlateLocation = new CarColorPlateLocation();

    svm = SVM::load(svm_model);
    //参数1的宽-参数2的宽 结果与参数3的余数为0  高也一样
    svmHog = new HOGDescriptor(Size(128,64),Size(16,16),Size(8,8),Size(8,8),3);
}


CarPlateRecgnize::~CarPlateRecgnize() {
    //释放
    if (!sobelPlateLocation) {
        delete sobelPlateLocation;
        sobelPlateLocation = 0;
    }

    if (!colorPlateLocation) {
        delete colorPlateLocation;
        colorPlateLocation = 0;
    }

    svm->clear();
    svm.release();
    if (svmHog) {
        delete svmHog;
        svmHog = 0;
    }
}

/*
*	 识别车牌 返回结果给调用者
*		1、定位
*		2、识别
*/
string CarPlateRecgnize::plateRecgnize(Mat src) {

    vector< Mat > sobel_plates;
    //sobel定位
    sobelPlateLocation->location(src, sobel_plates);

    //颜色定位
    vector< Mat > color_plates;
    colorPlateLocation->location(src, color_plates);

    vector< Mat > plates;
    //把sobel_plates的内容 全部加入plates向量
    plates.insert(plates.end(),sobel_plates.begin(), sobel_plates.end());
    plates.insert(plates.end(), color_plates.begin(), color_plates.end());

    int index = -1;
    float minScore = FLT_MAX; //float的最大值
    //使用 svm 进行 评测
    for (int i = 0;i< plates.size();++i)
    {
        Mat plate = plates[i];
        //先灰度化，再二值化，灰度化只剩下一个通道
        Mat gray;
        cvtColor(plate, gray,COLOR_BGR2GRAY);
        //二值化 必须是以单通道进行
        Mat shold;
        threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
        //提取特征
        Mat features;
        getHogFeatures(svmHog, shold, features);
        //features 进行转化，把数据保存成一行
        Mat samples = features.reshape(1,1);
        //转化数据存储格式
        samples.convertTo(samples, CV_32FC1 );

        //原始模式
        // svm: 直接告诉你这个数据是属于什么类型.
        // RAW_OUTPUT：让svm 给出一个评分
//        char name[100];
//        sprintf(name, "候选车牌%d", i);
//        imshow(name, plate);

        float score = svm->predict(samples, noArray(), StatModel::Flags::RAW_OUTPUT);
        printf("评分：%f\n",score);
        if (score < minScore) {
            minScore = score;
            index = i;
        }

        gray.release();
        shold.release();
        features.release();
        samples.release();
    }

    Mat dst;
    if (index >= 0) {
        dst = plates[index].clone();
    }
//    imshow("车牌", dst);
//    waitKey();
//    释放
    for (Mat p : plates) {
        p.release();
    }
    return string("123");
}



void CarPlateRecgnize::getHogFeatures(HOGDescriptor *hog, Mat src, Mat &out) {
    //重新定义大小 缩放 提取特征的时候数据需要为  ：CV_32S 有符号的32位数据
    Mat trainImg = Mat(hog->winSize, CV_32S);
    resize(src, trainImg, hog->winSize);
    vector<float> d;
    hog->compute(trainImg, d, Size(8,8));

    Mat features(d);
    //特征矩阵
    features.copyTo(out);
    features.release();
    trainImg.release();
}
