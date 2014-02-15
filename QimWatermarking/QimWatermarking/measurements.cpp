//
//  measurements.cpp
//  Image Similarity Measurement
//
//  Created by C��dric Golmard on 04/11/2013.
//  Copyright (c) 2013 C��dric Golmard. All rights reserved.
//

#include "measurements.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void convertRGBtoIntensity(Mat& imRGB, Mat& imIntensity);

///////////////////////////
//  Pixel-based measures //
///////////////////////////

//  Peak Signal-to-Noise Ratio
double getPSNR(Mat& im1, Mat& im2)
{
    //  The measure is computed only from the intensity
    // RGB to Intensity
    Mat v1;
    Mat v2;
    convertRGBtoIntensity(im1, v1);
    convertRGBtoIntensity(im2, v2);
    
    // Sum square error
    Mat s1;
    absdiff(v1, v2, s1);        //  |im1 - im2|
    s1.convertTo(s1, CV_32F);   //  else cannot make a square on 8 bits
    s1 = s1.mul(s1);            //  |im1 - im2|^2
    Scalar s = sum(s1);         //  sum elements per channel
    double sse = s.val[0] + s.val[1] + s.val[2];    //  sum channels
    
    // Mean square error
    double mse  = sse / (double)(im1.channels() * im1.total());
    double psnr = 10.0 * log10(255*255/mse);
    return psnr;
}

//  Image Fidelity
double getIF(Mat& im1, Mat& im2)
{
    //  The measure is computed only from the intensity
    // RGB to Intensity
    Mat v1;
    Mat v2;
    convertRGBtoIntensity(im1, v1);
    convertRGBtoIntensity(im2, v2);
    
    // Sum square error
    Mat s1;
    absdiff(v1, v2, s1);        //  |im1 - im2|
    s1.convertTo(s1, CV_32F);   //  else cannot make a square on 8 bits
    s1 = s1.mul(s1);            //  |im1 - im2|^2
    Scalar sum1 = sum(s1);      //  sum elements per channel
    double sse = sum1.val[0];   //  + sum1.val[1] + sum1.val[2]; // sum channels
    
    // Sum square of reference image
    Mat s2;
    s2.convertTo(s2, CV_32F);   //  else cannot make a square on 8 bits
    s2= v1.mul(v1);             //  (im2)^2
    Scalar sum2 = sum(s2);      //  sum elements per channel
    double ssref = sum2.val[0]; //  + sum2.val[1] + sum2.val[2]; // sum channels
    
    // Image Fidelidy
    return 1 - sse/ssref;
}

//////////////////////////////////
//  Correlation-based measures  //
//////////////////////////////////

//  Normalized Cross Correlation
double getNCC(Mat& im1, Mat& im2){
    
    //  The measure is computed only from the intensity
    //  RGB to Intensity
    Mat v1;
    Mat v2;
    convertRGBtoIntensity(im1, v1);
    convertRGBtoIntensity(im2, v2);
    
    //  Mean  and standard deviation values
    Scalar mean_im1,mean_im2;
    Scalar stddev_im1, stddev_im2, stddev_prod;
    
    meanStdDev(v1, mean_im1, stddev_im1);
    meanStdDev(v2, mean_im2, stddev_im2);
    multiply(stddev_im1, stddev_im2, stddev_prod);
    
    subtract(v1, mean_im1, v1);     //  im1 - mean(im1)
    subtract(v2, mean_im2, v2);     //  im2 - mean(im2)
    v1 = v1.mul(v2);                //  (im1 - mean(im1))(im2 - mean(im2))
    Scalar sum1 = sum(v1);          //  sum elements per channel
    divide(sum1, stddev_prod, sum1);
    
    double ssum = sum1.val[0];      //  + sum1.val[1] + sum1.val[2];      // sum channels
    int nb_pixels = v1.cols * v1.rows;
    
    return ssum/nb_pixels;
}


//  Structural SIMilarity
/*
 double getSSIM(Mat &im1, Mat &im2){
 
 //  RGB to Intensity
 Mat v1;
 Mat v2;
 convertRGBtoIntensity(im1, v1);
 convertRGBtoIntensity(im2, v2);
 
 // Constants
 double k1 = 0.01, k2 = 0.03;
 int L = 255;
 double c1 = pow(k1*L, 2);
 double c2 = pow(k2*L, 2);
 double c3 = c2/2;
 
 double mean1=0, mean2=0;
 double var1=0, var2=0, cov12=0;
 mean1 = mean(v1).val[0];
 mean2 = mean(v2).val[0];
 */
/*
 Mat mean1, covar1, var1;
 Mat mean2, covar2, var2;
 calcCovarMatrix(v1, covar1, mean1, CV_COVAR_NORMAL);
 calcCovarMatrix(v2, covar2, mean2, CV_COVAR_NORMAL);
 */
/*
 double A = 2*mean1*mean2+c1;
 double B = 2*cov12+c2;
 double C = pow(mean1, 2) * pow(mean2, 2) + c1;
 double D = pow(var1, 2) * pow(var2, 2) + c1;
 return (A*B)/(C*D);
 */
/*
 double luminance=0, constrast=0, structure=0;
 luminance = (2*mean1*mean2+c1)/(pow(mean1, 2)+pow(mean2, 2)+c1);
 constrast = (2*var1*var2+c1)/(pow(var1, 2)+pow(var2, 2)+c1);
 structure = (2*cov12+c3)/(var1*var2+c3);
 return luminance*constrast*structure;
 }
 */

//  Structural SIMilarity
double getSSIM(Mat& im1, Mat& im2)
{
    //  The measure is computed only rom the intensity
    //  RGB to Intensity
    Mat v1;
    Mat v2;
    convertRGBtoIntensity(im1, v1);
    convertRGBtoIntensity(im2, v2);
    
    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/
    int d     = CV_32F;
    
    Mat I1, I2;
    v1.convertTo(I1, d);           // cannot calculate on one byte large values
    v2.convertTo(I2, d);
    
    Mat I2_2   = I2.mul(I2);        // I2^2
    Mat I1_2   = I1.mul(I1);        // I1^2
    Mat I1_I2  = I1.mul(I2);        // I1 * I2
    
    /***********************PRELIMINARY COMPUTING ******************************/
    
    Mat mu1, mu2;   //
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);
    
    Mat mu1_2   =   mu1.mul(mu1);
    Mat mu2_2   =   mu2.mul(mu2);
    Mat mu1_mu2 =   mu1.mul(mu2);
    
    Mat sigma1_2, sigma2_2, sigma12;
    
    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;
    
    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;
    
    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;
    
    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3;
    
    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    
    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
    
    Mat ssim_map;
    divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;
    
    // a similarity index for each channel of the image.
    Scalar mssim = mean( ssim_map ); // mssim = average of ssim map
    //cout << mssim << endl;
    
    //  The measure is computed only rom the intensity (we used a 1-channel image)
    return mssim.val[0];
}

void convertRGBtoIntensity(Mat& imRGB, Mat& imIntensity){
    // RGB to HSV
    Mat im1_HSV;
    cvtColor(imRGB, im1_HSV, CV_RGB2HSV);
    vector<Mat> channels1(3);
    split(im1_HSV, channels1);
    
    // get the V (Value = Intensity) channel for the image.
    imIntensity = channels1[2];
}
