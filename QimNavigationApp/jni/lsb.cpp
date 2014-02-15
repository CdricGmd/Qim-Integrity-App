#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "lsb.hpp"

using namespace std;
using namespace cv;

void insert_lsb(Mat& image, vector<int> mark);
void extract_lsb(Mat& image, vector<int>& mark);
void MarkToBinayImage(vector<int> mark, Mat& bintatoo);

void insert_lsb(Mat& image, vector<int> mark) {

	// get a binary mask
	Mat tatoo;
	resize(tatoo, tatoo, image.size());
	MarkToBinayImage(mark, tatoo);

	// Intensity
	Mat image_HSV;
	Mat imIntensity;
	cvtColor(image, image_HSV, CV_RGB2HSV);
	vector<Mat> hsv_channel(3);
	split(image_HSV, hsv_channel);
	imIntensity = hsv_channel[2].clone();

	//  LSB
	// (8-1 =) 110 & XYZ = XY0;
	Mat notlsb_mask = 254
			* Mat::ones(imIntensity.rows, imIntensity.cols, CV_8U);
	bitwise_and(imIntensity, notlsb_mask, imIntensity);
	add(imIntensity, tatoo, imIntensity);

	// Save RGB image
	hsv_channel[2] = imIntensity.clone();
	merge(hsv_channel, image_HSV);
	// image_w = Mat(image.size().height, image.size().width, image.type());
	cvtColor(image_HSV, image, CV_HSV2RGB);
}

void extract_lsb(Mat& image, vector<int>& mark) {

	// get the V (Value = Intensity) channel for the image.
	Mat image_HSV;
	Mat imIntensity;
	cvtColor(image, image_HSV, CV_RGB2HSV);
	vector<Mat> hsv_channel(3);
	split(image_HSV, hsv_channel);
	imIntensity = hsv_channel[2];
	//hsv_channel[2].convertTo(imIntensity, CV_8U);

	for (size_t i = 0; i < imIntensity.rows; i++)
			for (size_t j = 0; j < imIntensity.cols; j++)
				mark.push_back(imIntensity.at<uchar>(j, i) & 01);
}

void MarkToBinayImage(vector<int> mark, Mat& bintatoo) {
	bintatoo.convertTo(bintatoo, CV_8U);
	int index = 0;
	int marksize = mark.size();
	for (size_t i = 0; i < bintatoo.rows; i++)
		for (size_t j = 0; j < bintatoo.cols; j++) {
			bintatoo.at<uchar>(j, i) = mark[index % marksize] & 01;
			index++;
		}
}
