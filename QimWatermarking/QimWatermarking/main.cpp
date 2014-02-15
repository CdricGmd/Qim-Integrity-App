//
//  main.cpp
//  QimWatermarking
//
//  Created by Cédric Golmard on 20/01/2014.
//  Copyright (c) 2014 Cédric Golmard. All rights reserved.
//

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#include <opencv2/opencv.hpp>

#include "mQim.hpp"
#include "measurements.hpp"
using namespace std;

void insertionMenu();
void detectionMenu();
void insertion(int insertion_method);
void displayMeasures();
double detection();
double diffclock(clock_t clock1, clock_t clock2);
//
void doRequantize(int nbColors);
void QuantizeImage(Mat& im, Mat& im_out, int delta);


string im_path;
string im_w_path;
string key_path;
int delta = 150;
double alpha = 0.5;

int main(int argc, const char * argv[])
{
    cout << endl;
    cout << "" << endl;
    cout << " Welcome to Qim watermarking command-ine tool. " << endl;
    cout << " usage : QimWatermarking image image_w key  [DIRECT for direct mode] " << endl;
    cout << "" << endl << endl;
    
    if (argc<4) {
        return 0;
    }
    
    // File paths
    im_path = argv[1];
    im_w_path = argv[2];
    key_path = argv[3];
    
    
    // ALTERNATIVE
    if(argc >= 5){
        string mode(argv[4]);
        if(mode == "DIRECT_MARK"){
            insertion(2);
            detectionMenu();
            displayMeasures();
        }else if(mode == "DIRECT_CHECK"){
            detectionMenu();
        }else if(mode == "REQUANTIZE"){
            if (argc >= 6) {
                int nDivColors = 1;
                stringstream conv;
                conv << argv[5];
                conv >> nDivColors;
                insertion(2);
                doRequantize(nDivColors);
                detectionMenu();
                displayMeasures();
            }
        }
        return 0;
    }
    
    // Menu
    int menu_choice = 0;
    while(true){
        cout << endl;
        cout << "___________"<< endl;
        cout << " MAIN MENU " << endl;
        cout << " Activity: " << endl;
        cout << " - 1: mark image " << endl;
        cout << " - 2: detect mark "<< endl;
        cout << " - 3: measures " << endl;
        cout << " - else: quit "<< endl;
        cout << "> choice: ";
        cin >> menu_choice;
        
        switch (menu_choice) {
            case 1:
                insertionMenu();
                break;
            case 2:
                detectionMenu();
                break;
            case 3:
                displayMeasures();
                break;
            default:
                return 0;
                break;
        }
        menu_choice = 0;
    }
    
    return 0;
}

void insertionMenu(){
    
    int insertion_method = 0;
    
    cout << endl;
    cout << " MARK IMAGE " << endl;
    cout << " Insertion method: " << endl;
    cout << " - 1: random blocks " << endl;
    cout << " - 2: prior detection criteria " << endl;
    cout << " - 3: energy criteria " << endl;
    cout << " - else: back to menu " << endl;
    cout << " > choice: ";
    cin >> insertion_method;
    switch (insertion_method) {
        case 1:
        case 2:
        case 3:
            insertion(insertion_method);
            break;
        default:
            return;
            break;
    }
    insertion_method = 0;
}

void detectionMenu(){
    //cout << endl;
    cout << "___________"<< endl;
    cout << " DETECTION " << endl;
    double score = detection();
    cout << endl;
    cout << " DETECTION SCORE: " << score << endl;
    cout << " Score is % of similarity between generated and extracted marks." << endl;
}

double diffclock(clock_t clock1, clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
    return diffms;
}

void insertion(int insertion_method){
    //cout << endl;
    cout << "___________"<< endl;
    cout << " INSERTION " << endl;
    
    cout << " Load image. " << im_path <<endl;
    Mat image = imread(im_path, CV_LOAD_IMAGE_COLOR);
    if(!image.data) {
		cout << " //// No image data !" << endl;
        return;
    }
    
    vector<int> mark;
    vector<double> keys;
    vector<int> allcols, allrows;
    
    cout << " Generate mark based on content... ";
    getContentMarkBinary(image, mark, 8);
    cout << " Mark size: " << mark.size() << endl;
    
    cout << " Inserting mark in image...";
    clock_t start = clock();
    switch (insertion_method) {
        case 1:
            // RANDOM
            insertionRandom(image, mark, keys, allcols, allrows, alpha, delta);
            break;
        case 2:
            // DETECTION
            insertionPriorDetectionCriteria(image, mark, keys, allcols, allrows, alpha, delta);
            break;
        case 3:
            // ENERGY
            insertionPriorEnergyCriteria(image, mark, keys, allcols, allrows, alpha, delta);
            break;
        default:
            return;
            break;
    }
    cout << " done (" << diffclock(clock(), start) <<" ms)" << endl;
    
    cout << " Saving marked image...";
    imwrite(im_w_path, image);
    cout << endl;
    
    cout << " Saving key file...";
    ofstream ostr(key_path.data());
    writeImageKeyFile(ostr, keys, allcols, allrows);
    ostr.close();
    cout << endl;
}

double detection(){
    
    cout << " Load marked image. " << endl;
    Mat image = imread(im_w_path, CV_LOAD_IMAGE_COLOR);
    if(!image.data) {
		cout << " //// No image data !" << endl;
        return 0;
    }
    
    vector<int> mark_content;
    vector<int> mark_inserted;
    vector<double> keys;
    vector<int> allcols, allrows;
    
    cout << " Generate mark based on content... ";
    getContentMarkBinary(image, mark_content, 8);
    int marksize = (int) mark_content.size();
    cout << " Mark size: " << marksize << endl;
    
    cout << " Reading key file...";
    ifstream istr(key_path.data());
    readImageKeyFile(istr, keys, allcols, allrows);
    istr.close();
    cout << endl;
    
    cout << " Detecting mark in image...";
    clock_t start = clock();
    detection(image, mark_inserted, keys, allcols, allrows, alpha, delta);
    cout << " done (" << diffclock(clock(), start) <<" ms)" << endl;
    
    cout << " Comparing marks... ";
    int goodmatches = compareMarks(mark_content, mark_inserted);
    cout << goodmatches << "/" << marksize << endl;
    
    return (double) goodmatches / marksize;
}

void displayMeasures(){
    Mat image = imread(im_path, CV_LOAD_IMAGE_COLOR);
    Mat image_w = imread(im_w_path, CV_LOAD_IMAGE_COLOR);
    
    cout << "______________" << endl;
    cout << " MEASUREMENTS " << endl;
    cout << " PSNR: " << getPSNR(image, image_w) << endl;
    cout << " SSIM: " << getSSIM(image, image_w) << endl;
    cout << " NCC:  " << 0<<endl;//getNCC(image, image_w) << endl;
    cout << " IF:   " << 0<<endl;//getIF(image, image_w) << endl;
    cout << endl;
    
}

void doRequantize(int divColors){
    if(divColors<1)
        return;
    
    Mat image_w = imread(im_w_path, CV_LOAD_IMAGE_COLOR);
    Mat image_w_req;
    
    //const int MAX_COLORS = 16581375; //255 * 255 * 255 = 16581375
    //int nbColorsTrue = min(nbColors, min(image_w.cols*image_w.rows -1, MAX_COLORS));
    
    QuantizeImage(image_w, image_w_req, divColors);
    imwrite(im_w_path, image_w_req);
}

void QuantizeImage(Mat& im, Mat& im_out, int ndiv){
    
    int nrow = im.rows;
    int ncol = im.cols;
    /*
    // Sample matrix for clustering. 2D array : each row is an rgb pixel
    
    vector<Mat> planes;
    split(im, planes);
    Mat rgb(nrow*ncol,(int)planes.size(),CV_32F);
    for (int t=0; t<planes.size(); t++) {
        planes[t].convertTo(planes[t], CV_32F);
        planes[t].reshape(0, rgb.rows).copyTo(rgb.col(t));
    }
    
    // Requantize using k-means clustering
    int attemps = 1, maxiter = 3;//1 3
    double epsilon = 1.0;
    Mat centers(delta,1, CV_32FC3); // Mat for the centers of the clusters
    Mat labels(nrow*ncol, 1, CV_32S); // Mat for the labels (center's indices) of each pixel
    TermCriteria termcrit(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, maxiter, epsilon);
    kmeans(rgb, delta, labels, termcrit, attemps, KMEANS_RANDOM_CENTERS, centers);
    
    im_out = Mat(nrow,ncol,CV_8UC3);
    // fill the result image with the center's value corresponding each pixel's label.
    for (int i = 0; i<nrow; i++){
        for (int j = 0; j<ncol; j++){
            Vec3b& pixel = im_out.at<Vec3b>(i,j); // select the pixel
            int label = labels.at<int>(i*nrow +j,0); // get cluster's label for this pixel
            for (int k = 0; k<3;k++){
                pixel[k] = static_cast<unsigned char>(centers.at<float>(label,k));// get center's value
            }
        }
    }

    */
    
    
    
    Mat  a;
    int nchannel=im.channels();
    im.convertTo(a, CV_8UC(nchannel));
    //int ndiv=4;
    
    double x1,x2;
    cv::minMaxLoc(a,&x1,&x2,NULL,NULL);
    int div=x2/ndiv;
    a.convertTo(im_out,a.type(),1.0/div,0);
    im_out.convertTo(im_out,im_out.type(),div,0);

}
