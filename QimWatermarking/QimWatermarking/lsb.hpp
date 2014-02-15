//
//  lsb.hpp
//

#ifndef Image_Similarity_Measurement_lsb_h
#define Image_Similarity_Measurement_lsb_h

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

void insert_lsb(Mat& image, vector<int> mark);
void extract_lsb(Mat& image, vector<int>& mark);

#endif
