cmake_minimum_required (VERSION 3.8)

project(OpencvCarRecgnize)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)

set(Opencv_include "/usr/local/Cellar/opencv/3.4.2/include" )

set(Opencv_lib "/usr/local/Cellar/opencv/3.4.2/lib")

#引入头文件
include_directories(${Opencv_include})

#引入库文件
link_directories(${Opencv_lib})

# 将源代码添加到此项目的可执行文件。
add_executable (OpencvCarRecgnize "CarRecgnize.cpp" CarPlateLocation.cpp CarPlateRecgnize.cpp )

set(OpenCV_LIBS ippicv opencv_line_descriptor ippiw opencv_ml ittnotify opencv_objdetect libjpeg-turbo opencv_optflow libprotobuf opencv_phase_unwrapping libwebp opencv_photo opencv_aruco opencv_plot opencv_bgsegm opencv_reg opencv_bioinspired opencv_rgbd opencv_calib3d opencv_saliency opencv_ccalib opencv_shape opencv_core opencv_stereo opencv_datasets opencv_stitching opencv_dnn opencv_structured_light opencv_dnn_objdetect opencv_superres opencv_dpm opencv_surface_matching opencv_face opencv_tracking opencv_features2d opencv_video opencv_flann opencv_videoio opencv_fuzzy  opencv_videostab opencv_hfs opencv_xfeatures2d opencv_highgui opencv_ximgproc opencv_img_hash opencv_xobjdetect opencv_imgcodecs opencv_xphoto opencv_imgproc)

target_link_libraries( OpencvCarRecgnize ${OpenCV_LIBS})