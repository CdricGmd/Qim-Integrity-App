//
//  measurements.h
//  Image Similarity Measurement
//
//  Created by Cédric Golmard on 04/11/2013.
//  Copyright (c) 2013 Cédric Golmard. All rights reserved.
//
//  Similarity measures between images using openCV.

#ifndef __Image_Similarity_Measurement__measurements__
#define __Image_Similarity_Measurement__measurements__

#include <opencv2/opencv.hpp>
using namespace cv;

//////////////////////////////////
//     Pixel-based measures     //
//////////////////////////////////

//  Peak Signal-to-Noise Ratio
//  PSNR
//  Result expected : score from 0.0 to infinity
//      * perfect fit           : infinity
//      * very good similarity  : > 50
//      * good similarity       : > 30
double getPSNR(Mat &im1, Mat &im2);

//  Image Fidelity
//  IF
//  Result expected :
//      * same images           : 1
double getIF(Mat &im1, Mat &im2);

//////////////////////////////////
//  Correlation-based measures  //
//////////////////////////////////

//  Normalized Cross Correlation
//  NCC
//  Result expected :
//      * same images           : 1
double getNCC(Mat &im1, Mat &im2);

//  Structural SIMilarity
//  SSIM
//  Result expected :index between 0.0 and 1.0
//      * perfect fit           : 1
//      * no similarity         : 0
double getSSIM(Mat &im1, Mat &im2);

#endif /* defined(__Image_Similarity_Measurement__measurements__) */
