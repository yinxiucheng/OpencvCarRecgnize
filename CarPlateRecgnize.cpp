//
// Created by xiucheng yin on 2018/11/15.
//
#include "CarPlateRecgnize.h"

string CarPlateRecgnize::ZHCHARS[] = { "川","鄂","赣","甘","贵","桂","黑","沪","冀","津","京","吉","辽","鲁","蒙","闽","宁","青","琼","陕","苏","晋","皖","湘","新","豫","渝","粤","云","藏","浙" };
char CarPlateRecgnize::CHARS[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z' };

CarPlateRecgnize::CarPlateRecgnize(const char *svm_model, const char* ann_ch_path, const char* ann_path) {
    sobelPlateLocation = new CarSobelPlateLocation();
    colorPlateLocation = new CarColorPlateLocation();

    svm = SVM::load(svm_model);
    //参数1的宽-参数2的宽 结果与参数3的余数为0  高也一样
    svmHog = new HOGDescriptor(Size(128, 64), Size(16, 16), Size(8, 8), Size(8, 8), 3);

    //创建特征提取对象
    annHog = new HOGDescriptor(Size(32, 32), Size(16, 16), Size(8, 8), Size(8, 8), 3);
    //识别汉字
    annCh = ANN_MLP::load(ann_ch_path);
    //识别模型
    ann = ANN_MLP::load(ann_path);
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
    if (annHog) {
        delete annHog;
        annHog = 0;
    }
    annCh->clear();
    annCh.release();
    ann->clear();
    ann.release();
}

/*
*	 识别车牌 返回结果给调用者
*		1、定位
*		2、识别
*/
string CarPlateRecgnize::plateRecgnize(Mat src) {

    vector<Mat> sobel_plates;
    //sobel定位
    sobelPlateLocation->location(src, sobel_plates);

    //颜色定位
    vector<Mat> color_plates;
    colorPlateLocation->location(src, color_plates);

    vector<Mat> plates;
    //把sobel_plates的内容 全部加入plates向量
    plates.insert(plates.end(), sobel_plates.begin(), sobel_plates.end());
    plates.insert(plates.end(), color_plates.begin(), color_plates.end());

    int index = -1;
    float minScore = FLT_MAX; //float的最大值
    //使用 svm 进行 评测
    for (int i = 0; i < plates.size(); ++i) {
        Mat plate = plates[i];
        //先灰度化，再二值化，灰度化只剩下一个通道
        Mat gray;
        cvtColor(plate, gray, COLOR_BGR2GRAY);
        //二值化 必须是以单通道进行
        Mat shold;
        threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
        //提取特征
        Mat features;
        getHogFeatures(svmHog, shold, features);
        //features 进行转化，把数据保存成一行
        Mat samples = features.reshape(1, 1);
        //转化数据存储格式
        samples.convertTo(samples, CV_32FC1);

        //原始模式
        // svm: 直接告诉你这个数据是属于什么类型.
        // RAW_OUTPUT：让svm 给出一个评分
//        char name[100];
//        sprintf(name, "候选车牌%d", i);
//        imshow(name, plate);

        float score = svm->predict(samples, noArray(), StatModel::Flags::RAW_OUTPUT);
        printf("评分：%f\n", score);
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

    //imshow("车牌", dst);
    //识别 ...... ann ：神经网络
    Mat plate_gray;
    cvtColor(dst, plate_gray, COLOR_BGR2GRAY);

    //二值化
    Mat plate_shold;
    threshold(plate_gray, plate_shold, 0, 255, THRESH_OTSU + THRESH_BINARY);

//    imshow("黑白", plate_shold);
    //去掉 车牌在车子上的固定点
    //因为固定点所在的行 它的黑到白、白到黑的改变次数是最小的
    clearFixPoint(plate_shold);
//    imshow("去干扰", plate_shold);

    vector<vector<Point>> contours;
    findContours(plate_shold, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    vector<Rect> charVec;
    for (vector<Point> point:contours) {
        Rect rect = boundingRect(point);
        //获得候选字符的图片
        Mat p = plate_shold(rect);

        //进行初步筛选
        if (verityCharSize(p)) {
            charVec.push_back(rect);
        }
    }
    //集合中仍然会存在 非字符矩形
    //对集合中的矩形按照x进行一下排序，保证它们是从左到右的顺序
    sort(charVec.begin(), charVec.end(), [](const Rect &r1, const Rect &r2) {
        return r1.x < r2.x;
    });

    //汉字比较特殊。 如何拿汉字？
    //获取城市字符所在下标，比如湘A，那么A就是城市字符，代表长沙
    int cityIndex = getCityIndex(charVec);

    //通过城市的下标 判断获取汉字轮廓
    Rect chineseRect;
    getChineseRect(charVec[cityIndex], chineseRect);

    //集合里面首先加入汉字
    vector<Mat> plateChar;
    plateChar.push_back(plate_shold(chineseRect));

    //从城市开始加入字符
    int count = 0;
    for (size_t j = cityIndex; j < charVec.size(), count<6; count++, ++j) {
        plateChar.push_back(plate_shold(charVec[j]));
    }
//    for(Mat s: plateChar){
//        imshow("", s);
//        waitKey();
//    }
//    rectangle(dst, chineseRect, Scalar(255, 0, 255));
//    imshow("省份汉字", dst);

    String plate_str;
    predict(plateChar, plate_str);

    plate_gray.release();
    plate_shold.release();

    return plate_str;
}


void CarPlateRecgnize::getHogFeatures(HOGDescriptor *hog, Mat src, Mat &out) {
    //重新定义大小 缩放 提取特征的时候数据需要为  ：CV_32S 有符号的32位数据
    Mat trainImg = Mat(hog->winSize, CV_32S);
    resize(src, trainImg, hog->winSize);
    vector<float> d;
    hog->compute(trainImg, d, Size(8, 8));

    Mat features(d);
    //特征矩阵
    features.copyTo(out);
    features.release();
    trainImg.release();
}

//去螺丝钉的挂牌
void CarPlateRecgnize::clearFixPoint(Mat &src) {
    //最大改变次数
    int maxChange = 10;
    //一个集合统计每一行的跳变次数
    vector<int> c;
    for (size_t i = 0; i < src.rows; i++) {
        //记录这一行的改变次数
        int change = 0;
        for (size_t j = 0; j < src.cols - 1; j++) {
            //获得像素值
            char p = src.at<char>(i, j);
            //当前的像素点与下一个像素点值是否相同
            if (p != src.at<char>(i, j + 1)) {
                change++;
            }
        }
        c.push_back(change);
    }

    for (size_t i = 0; i < c.size(); i++) {
        //取出每一行的改变次数
        int change = c[i];
        //如果小与max ，则可能就是干扰点所在的行
        if (change <= maxChange) {
            //把这一行都抹黑
            for (size_t j = 0; j < src.cols; j++) {
                src.at<char>(i, j) = 0;
            }
        }
    }
}

//通过宽高比 初步赛选。
bool CarPlateRecgnize::verityCharSize(Mat src) {
    //最理想情况 车牌字符的标准宽高比
    float aspect = 45.0f / 90;
    //当前的真是的宽高比
    float realAspect = (float)src.cols / src.rows;
    //最小的字符高
    float minHeight = 10.0f;
    //最大的字符高
    float maxHeight = 35.0f;
    //1. 判断高符合范围 2.宽、高比符合范围
    float error = 0.7f;

    float maxAspect = aspect + aspect * error;
    float minAspect = aspect - aspect * error;

    //这里只判断高，因为字符 1 宽度太窄了。
    if (realAspect >= minAspect && realAspect <= maxAspect && src.rows >= minHeight && src.rows <= maxHeight) {
        return true;
    }
    return false;
}


//通过拿到城市的轮廓取到 省份汉字的轮廓
int CarPlateRecgnize::getCityIndex(vector<Rect> src) {
    int cityIndex = 0;
    for (int i = 0; i < src.size(); ++i) {
        Rect rect = src[i];
        //获得矩形
        //把车牌区域划分为7个字符
        //如果当前获得的矩形 它的中心点 比 1/7 大，比2/7小，那么就是城市的轮廓
        int midX = rect.x + rect.width / 2;
        if (midX < 136 / 7 * 2 && midX > 136 / 7) {
            cityIndex = i;
            break;
        }
    }
    return cityIndex;
}

//获取省份汉字框
void CarPlateRecgnize::getChineseRect(Rect city, Rect &chineseRect) {
    //把宽度稍微扩大一点
    float width = city.width * 1.15f;
    //城市轮廓的x坐标
    int x = city.x;

    int newX = x - width;
    chineseRect.x = newX >= 0 ? newX : 0;
    chineseRect.y = city.y;
    chineseRect.width = width;
    chineseRect.height = city.height;
}

//通过ANN进行识别
void CarPlateRecgnize::predict(vector<Mat> vector, String &result) {
    for (size_t i = 0; i < vector.size(); ++i) {
        Mat src = vector[i];
        Mat features;
        //
        getHogFeatures(annHog, src, features);
        Mat response;
        Point maxLoc;
        Point minLoc;

        Mat samples = features.reshape(1, 1);
        if (i){
            //识别字母和数字
            ann->predict(samples,response);
            //获取最大可信度 匹配度最高的属于31种中的哪一个。
            minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
            //跟你训练时候有关
            int index = maxLoc.x;
            result += CHARS[index];
        }else{
            //识别汉字
            annCh->predict(samples, response);
            minMaxLoc(response, 0, 0, 0, &maxLoc);
            //跟你训练的有关
            int index = maxLoc.x;
            //识别出来的汉字，拼到string当中去
            result += ZHCHARS[index];
        }
        cout << "匹配度:" << minLoc.x << endl;
    }
}
