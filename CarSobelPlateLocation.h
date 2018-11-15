//
// Created by xiucheng yin on 2018/11/15.
//

#ifndef OPENCVFACE_CARPLATELOCATION_H
#define OPENCVFACE_CARPLATELOCATION_H

#include "CarPlateLocation.h"


class CarSobelPlateLocation: public CarPlateLocation{
public:
    CarSobelPlateLocation();
    ~CarSobelPlateLocation();

    void location(Mat mat, vector<Mat>& dst);
};


#endif //OPENCVFACE_CARPLATELOCATION_H
