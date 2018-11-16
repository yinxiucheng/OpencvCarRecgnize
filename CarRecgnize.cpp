#include <iostream>
#include "CarPlateRecgnize.h"

int main() {
    CarPlateRecgnize p("/Users/xiuchengyin/Documents/Tina-NDK/OpencvCarRecgnize/resource/HOG_SVM_DATA2.xml",
    "/Users/xiuchengyin/Documents/Tina-NDK/OpencvCarRecgnize/resource/HOG_ANN_ZH_DATA2.xml",
    "/Users/xiuchengyin/Documents/Tina-NDK/OpencvCarRecgnize/resource/HOG_ANN_DATA2.xml");
    Mat src = imread("/Users/xiuchengyin/Documents/Tina-NDK/OpencvCarRecgnize/test_img/test11.jpg");
    p.plateRecgnize(src);
    cout << p.plateRecgnize(src) << endl;
    waitKey();
    return 0;
}