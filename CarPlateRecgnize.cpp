//
// Created by xiucheng yin on 2018/11/15.
//

#include "CarPlateRecgnize.h"

CarPlateRecgnize::CarPlateRecgnize() {
    sobelPlateLocation = new CarSobelPlateLocation();
    colorPlateLocation = new CarColorPlateLocation();
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

    return string("123");
}