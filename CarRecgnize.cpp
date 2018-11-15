#include <iostream>
#include "CarPlateRecgnize.h"

int main() {

    CarPlateRecgnize p;
    Mat src = imread("/Users/xiuchengyin/Documents/Tina-NDK/OpencvCarRecgnize/test_img/test2.jpg");
    p.plateRecgnize(src);
    waitKey();
    return 0;
}