//
// Created by xiucheng yin on 2018/11/15.
//

#ifndef OPENCVCARRECGNIZE_CARPLATERECGNIZE_H
#define OPENCVCARRECGNIZE_CARPLATERECGNIZE_H

#include "CarSobelPlateLocation.h"
#include "CarColorPlateLocation.h"
#include <string>

using namespace std;


class CarPlateRecgnize {
public:
    CarPlateRecgnize();
    ~CarPlateRecgnize();

    string plateRecgnize(Mat src);
private:
    CarSobelPlateLocation *sobelPlateLocation = 0;
    CarColorPlateLocation *colorPlateLocation = 0;

};


#endif //OPENCVCARRECGNIZE_CARPLATERECGNIZE_H
