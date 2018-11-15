//
// Created by xiucheng yin on 2018/11/15.
//

#include "CarPlateRecgnize.h"

CarPlateRecgnize::CarPlateRecgnize() {
    plateLocation = new CarPlateLocation();
}


CarPlateRecgnize::~CarPlateRecgnize() {
    //释放
    if (!plateLocation) {
        delete plateLocation;
        plateLocation = 0;
    }
}

/*
*	 识别车牌 返回结果给调用者
*		1、定位
*		2、识别
*/
string CarPlateRecgnize::plateRecgnize(Mat src) {

    Mat plate;
    //定位
    plateLocation->location(src, plate);

    return string("123");
}