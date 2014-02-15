//
//  mQim.cpp
//
//  Created by Cédric Golmard on 06/11/2013.
//
#include "mQim.hpp"

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <fstream>

#include "measurements.hpp"

using namespace std;
using namespace cv;

//////METHODS///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void insertionRandom(Mat& image, vector<int> mark, vector<double>& key,
                     vector<int>& pos_block_col, vector<int>& pos_block_row, double alpha,
                     int delta);
void insertionPriorDetectionCriteria(Mat& image, vector<int> mark,
                                     vector<double>& key, vector<int>& pos_block_col,
                                     vector<int>& pos_block_row, double alpha, int delta);
void insertionPriorEnergyCriteria(Mat& image, vector<int> mark,
                                  vector<double>& key, vector<int>& pos_block_col,
                                  vector<int>& pos_block_row, double alpha, int delta);

void detection(Mat& image, vector<int>& mark, vector<double> key,
               vector<int> pos_block_col, vector<int> pos_block_row, double alpha,
               int delta);

void getContentMarkBinary(Mat& image, vector<int>& mark, int dctblock_size);

int compareMarks(vector<int> mark1, vector<int> mark2);

void displayMeasures(Mat& image1, vector<int>& mark1, Mat& image2,
                     vector<int>& mark2);

template<class T>
bool writeNextRow(ostream& str, vector<T> rowdata);

template<class T>
bool readNextRow(istream& str, vector<T>& rowdata);

bool writeAllKeyFile(ostream& str, vector<int> framenums,
                     vector<vector<double> > allkeys, vector<vector<int> > allrows,
                     vector<vector<int> > allcols);
bool readAllKeyFile(istream& str, vector<int> &framenums,
                    vector<vector<double> > &allkeys, vector<vector<int> > &allrows,
                    vector<vector<int> > &allcols);
bool writeImageKeyFile(ostream& ostr, vector<double> keys, vector<int> cols,
                       vector<int> rows);
bool readImageKeyFile(istream& istr, vector<double>& keys, vector<int>& cols,
                      vector<int>& rows);

//////AUX

double quantize(double value, double delta);
double quantize(double value);

void insert_block(Mat& block, int mark, double key, double alpha, int delta);
void detect_block(Mat& block, int& mark, double key, double alpha, int delta);

double detection_measure(Mat& block, double key, double alpha, int delta);

//////VARS//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// default Qim parameters
int block_size = 4;
int delta_q = 150;

// Perception mask
float masque_t[4][4] = { { 0, 0.58, 1.2, 2.39 }, { 0.58, 1.12, 1.49, 2.3 }, {
    1.2, 1.49, 3.07, 4.35 }, { 2.39, 2.3, 4.35, 7.25 } };
double norm_masque = 11.4468;

// Files
char separator = ',';

//////INSERTION//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * insertionRandom
 *
 * Insert the mark at random block positions
 *
 * @param image
 * @param mark
 * @param key
 * @param pos_block_col
 * @param pos_block_row
 * @param alpha
 * @param delta
 */
void insertionRandom(Mat& image, vector<int> mark, vector<double>& key,
                     vector<int>& pos_block_col, vector<int>& pos_block_row, double alpha,
                     int delta) {
    
	/////////////////////////////////////////////////////////////////////////////
	// Work on a copy
	key.clear();
	pos_block_col.clear();
	pos_block_row.clear();
	double thiskey = 0;
	int block_col = 0, block_row = 0;
	Mat image_mark;
    
	srand((int) time(NULL)); // init for random values
    
	// RGB => HSV
	// image.convertTo(image_mark, CV_32FC3); //
	cvtColor(image, image, CV_BGR2YCrCb);
	int V = 0; // intensity = V = index #2
    
	// Split block in channels
	vector<Mat> planes;
	split(image, planes);
    
	for (size_t t = 0; t < planes.size(); t++)
		planes[t].convertTo(planes[t], CV_32FC1);
    
	/////////////////////////////////////////////////////////////////////////////
	// Image block processing : for each (block_size*block_size) block
	vector<int>::const_iterator mark_it(mark.begin()), mark_end(mark.end());
	for (; mark_it != mark_end; mark_it++) {
        
		// generate random value
		thiskey = (double) rand() / RAND_MAX;
		block_row = rand() % ((image.rows)/ block_size);
		block_col = rand() % ((image.cols)/ block_size);
        
		// Select the block on intensity channel (V) // x, y w, h
		Mat block = planes[V](
                              Rect(block_size * block_col, block_size * block_row, block_size,
                                   block_size));
		Mat dctblock;
        
		/////////////////////////////////////////////////////////////////////////////
		// direct DCT
		dct(block, dctblock);
        
		/////////////////////////////////////////////////////////////////////////////
		// insertion
		insert_block(dctblock, *mark_it % 2, thiskey, alpha, delta);
        
		// update
		key.push_back(thiskey);
		pos_block_col.push_back(block_col);
		pos_block_row.push_back(block_row);
        
		/////////////////////////////////////////////////////////////////////////////
		// inverse DCT
		// dct(dctblock, block, DCT_INVERSE);
		idct(dctblock, block);
	}
    
	// merge planes in block
	for (size_t t = 0; t < planes.size(); t++)
        planes[t].convertTo(planes[t], CV_8UC1);
	merge(planes, image);
    
    // HSV -> RGB
    cvtColor(image, image, CV_YCrCb2BGR);
}

/**
 * insertionPriorDetectionCriteria
 *
 * insert the mark in only blocks respecting a specific condition on the detection to be more robust to attack. The insertion is slower due to several attempts to match the criteria for each block.
 *
 * @param image
 * @param mark
 * @param key
 * @param pos_block_col
 * @param pos_block_row
 * @param alpha
 * @param delta
 */
void insertionPriorDetectionCriteria(Mat& image, vector<int> mark,
                                     vector<double>& key, vector<int>& pos_block_col,
                                     vector<int>& pos_block_row, double alpha, int delta) {
    
    /////////////////////////////////////////////////////////////////////////////
    // Work on a copy
    //Mat image_mark;
    pos_block_col.clear();
    pos_block_row.clear();
    key.clear();
    double thiskey = 0;
    
    srand((int) time(NULL)); // init for random values

    int numblockrows =image.rows/block_size;
    int numblockcols =image.cols/block_size;
    int numblocks = numblockrows * numblockcols;
    int maxSearchIter = 2*numblocks;
    bool criteria_shunted = false;
    
    // RGB => HSV
    // image.convertTo(image_mark, CV_32FC3); //
    cvtColor(image, image, CV_BGR2YCrCb);
    int V = 0; // intensity = V = index #2
    
    // Split block in channels
    vector<Mat> planes;
    split(image, planes);
    
    for (size_t t = 0; t < planes.size(); t++)
        planes[t].convertTo(planes[t], CV_32FC1);
    
    /////////////////////////////////////////////////////////////////////////////
    // Image block processing : for each (block_size*block_size) block
    int block_col = 0, block_row = 0;
    double k = (1 - alpha) * delta;
    vector<int>::const_iterator mark_it(mark.begin()), mark_end(
                                                                mark.end());
    for (; mark_it != mark_end; mark_it++) {
        // generate a key
        thiskey = (double) rand() / RAND_MAX;
        
        /////////////////////////////////////////////////////////////////////////////
        //// Try the insertion : we selec a random block and then check if it matches our selection condition
        int counter =0;
        while (true) {
            
            counter++;
            // The criteria is shunted for the rest of the image if no block were found
            if(!criteria_shunted && counter >= maxSearchIter){
                criteria_shunted = true;
            }
            
            block_col = rand() % (numblockcols);
            block_row = rand() % (numblockrows);
            
            // Select the block on intensity channel (V)
            // Rect(col, row, w, h)
            Mat block = planes[V](
                                  Rect(block_col * block_size, block_row * block_size,
                                       block_size, block_size));
            Mat dctblock;
            
            /////////////////////////////////////////////////////////////////////////////
            // direct DCT
            dct(block, dctblock);
            //double x = norm(dctblock);
            
            /////////////////////////////////////////////////////////////////////////////
            // insertion
            insert_block(dctblock, *mark_it % 2, thiskey, alpha, delta);
            
            /////////////////////////////////////////////////////////////////////////////
            //// CRITERIA : chose a block where the detection is easy : value in a narrow part of the decision interval
            //// Decision interval has a wideness ok k = (1 - alpha) * delta,
            //// we chose to reduce it around the extrem values in order to separate the decisions intervals
            //// NB: k/4 can be changed to k/3 (faster) or k/5 (etc) but higher division may not find matching blocks quickly.
            double D = detection_measure(dctblock, thiskey, alpha,
                                         delta);
            // t = mess/(alphabet-1)*DD
            double t = (*mark_it % 2) * k;
            if (criteria_shunted || abs(abs(D) - t) < k / 4) {
                
                /////////////////////////////////////////////////////////////////////////////
                // inverse DCT
                // => insertion accepted
                idct(dctblock, block);
                
                // update
                key.push_back(thiskey);
                pos_block_col.push_back(block_col);
                pos_block_row.push_back(block_row);
                
                //DEBUG cout << "Inserted after " << counter << " attempts."<< endl;
                break;
            }
        }
    }
    
    // merge planes in block
    for (size_t t = 0; t < planes.size(); t++)
        planes[t].convertTo(planes[t], CV_8UC1);
    merge(planes, image);
    
    // HSV -> RGB
    cvtColor(image, image, CV_YCrCb2BGR);
}

/**
 * insertionPriorEnergyCriteria
 *
 * insert the mark in only blocks respecting a specific condition on the blocks energy to be more robust to attack. The insertion is slower due to several attempts to match the criteria for each block.
 *
 * @param image
 * @param mark
 * @param key
 * @param pos_block_col
 * @param pos_block_row
 * @param alpha
 * @param delta
 */
void insertionPriorEnergyCriteria(Mat& image, vector<int> mark,
                                  vector<double>& key, vector<int>& pos_block_col,
                                  vector<int>& pos_block_row, double alpha, int delta) {
    
    /////////////////////////////////////////////////////////////////////////////
    // Work on a copy
      //Mat image_mark;
    pos_block_col.clear();
    pos_block_row.clear();
    key.clear();
    double thiskey = 0;

    srand((int) time(NULL)); // init for random values
    
    int numblockrows =image.rows/block_size;
    int numblockcols =image.cols/block_size;
    int numblocks = numblockrows * numblockcols;
    int maxSearchIter = 2*numblocks;
    bool criteria_shunted = false;

    
    // RGB => HSV
    // image.convertTo(image_mark, CV_32FC3); //
    cvtColor(image, image, CV_BGR2YCrCb);
    int V = 0; // intensity = V = index #2
    
    // Split block in channels
    vector<Mat> planes;
    split(image, planes);
    
    for (size_t t = 0; t < planes.size(); t++)
        planes[t].convertTo(planes[t], CV_32FC1);
    
    /////////////////////////////////////////////////////////////////////////////
    //// Image block processing : for each (block_size*block_size) block
    int block_col = 0, block_row = 0;
    //double k = (1 - alpha) * delta;
    vector<int>::const_iterator mark_it(mark.begin()), mark_end(
                                                                mark.end());
    for (; mark_it != mark_end; mark_it++) {
        // generate a key
        thiskey = (double) rand() / RAND_MAX;
        
        /////////////////////////////////////////////////////////////////////////////
        //// Try the insertion : we selec a random block and then check if it matches our selection condition
        int counter =0;
        while (true) {
            counter++;
            // The criteria is shunted for the rest of the image if no block were found
            if(!criteria_shunted && counter >= maxSearchIter){
                criteria_shunted = true;
            }
            
            block_col = rand() % (numblockcols);
            block_row = rand() % (numblockrows);
            
            // Select the block on intensity channel (V)
            // Rect(col, row, w, h)
            Mat block = planes[V](
                                  Rect(block_col * block_size, block_row * block_size,
                                       block_size, block_size));
            Mat dctblock;
            
            /////////////////////////////////////////////////////////////////////////////
            // direct DCT
            dct(block, dctblock);
            double x = norm(dctblock);
            
            /////////////////////////////////////////////////////////////////////////////
            // insertion
            insert_block(dctblock, *mark_it % 2, thiskey, alpha, delta);
            
            /////////////////////////////////////////////////////////////////////////////
            // check energy after insertion
            double y = norm(dctblock);
            Scalar mean_sc;
            Scalar stddev_sc;
            meanStdDev(dctblock, mean_sc, stddev_sc);
            double mean = mean_sc.val[0];
            double stddev = stddev_sc.val[0];
            
            /////////////////////////////////////////////////////////////////////////////
            // Selection condition
            // Energy criteria
            double D = 1 - x / y - mean;
            //cout << rdmblock << endl;
            //cout << mean << " | " << stddev << endl;
            //cout << " - " << abs(D) << " < " << stddev/(block_size*block_size) << endl;
            //// CRITERIA : Check an inergy criteria on the block
            if (criteria_shunted || abs(D) < stddev / 4) {
                
                /////////////////////////////////////////////////////////////////////////////
                // inverse DCT
                // => insertion accepted
                idct(dctblock, block);
                
                // update
                key.push_back(thiskey);
                pos_block_col.push_back(block_col);
                pos_block_row.push_back(block_row);
                
                //DEBUG cout << "Inserted after " << counter << " attempts."<< endl;
                break;
            }
        }
    }
    
    // merge planes in block
    for (size_t t = 0; t < planes.size(); t++)
        planes[t].convertTo(planes[t], CV_8UC1);
    merge(planes, image);
    
    // HSV -> RGB
    cvtColor(image, image, CV_YCrCb2BGR);
    /////////////////////////////////////////////////////////////////////////////
    /*
     //  Mark a copy of the image with the random mark
     Mat randomly_marked = image.clone();
     insertion(randomly_marked, randomBits, key, posx, posy, alpha, delta);
     
     
     // Cpnvert all Y channels to block dct
     // RGB => YCrCb
     int V=0; // V => index 2
     cvtColor(randomly_marked, randomly_marked, CV_BGR2YCrCb);
     Mat rdm_dct;
     dctblock_direct(randomly_marked, rdm_dct, block_size);
     vector<Mat> rdm_planes;
     split(rdm_dct, rdm_planes);
     
     Mat imageYCrCb;
     cvtColor(image, imageYCrCb, CV_BGR2YCrCb);
     
     Mat original_dct;
     dct_blocks(imageYCrCb, original_dct, block_size, 0);
     vector<Mat> ori_planes;
     split(original_dct, ori_planes);
     
     
     
     //
     pos_block_col.clear();
     pos_block_row.clear();
     int lastx=0, lasty=0;
     vector<int>::const_iterator vit(mark.begin()), vend(mark.end());
     bool found = false;
     for(;vit != vend; vit++){
     for(int i = lasty; i < original_dct.size().height - block_size; i+=block_size) {
     found = false;
     for (size_t j = lastx; j < original_dct.size().width - block_size; j+=block_size) {
     // block
     Mat rdmblock = rdm_planes[V](Rect(j, i, block_size, block_size));
     Mat iniblock = ori_planes[V](Rect(j, i, block_size, block_size));
     
     //  Check energies
     Scalar mean_sc;
     Scalar stddev_sc;
     meanStdDev(rdmblock, mean_sc, stddev_sc);
     float mean = mean_sc.val[0];
     float stddev = stddev_sc.val[0];
     
     // energy criteria
     float D = 1 - (norm(rdmblock)/norm(iniblock)) - mean;
     //cout << rdmblock << endl;
     //cout << mean << " | " << stddev << endl;
     //cout << " - " << abs(D) << " < " << stddev/(block_size*block_size) << endl;
     
     if ((randomBits[i*rdm_dct.cols/block_size +j]%2 == *vit%2) && (abs(D)< stddev/(block_size*block_size))) {
     
     pos_block_row.push_back(i);
     pos_block_col.push_back(j);
     //directly copy the marked block;
     rdmblock.copyTo(iniblock);
     
     // update and break loops
     lastx=j+1;
     lasty=i+1;
     found = true;
     break;
     }
     }
     if(found)
     break;
     }
     }
     
     // block CDT marked -> YCbCr -> BGR : image marked
     merge(ori_planes, original_dct);
     dctblock_inverse(original_dct, imageYCrCb, block_size);
     
     cvtColor(imageYCrCb, image, CV_YCrCb2BGR);
     */
}

//////DETECTION//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * detection
 *
 * detect and extract the mark inserted in the image using the keys and block positions information
 *
 * @param image
 * @param mark
 * @param key
 * @param pos_block_col
 * @param pos_block_row
 * @param alpha
 * @param delta
 */
void detection(Mat& image, vector<int>& mark, vector<double> key,
               vector<int> pos_block_col, vector<int> pos_block_row,
               double alpha, int delta) {
    
    // RGB => YCrCb
    Mat image_mark;        // = image.clone();
    cvtColor(image, image_mark, CV_BGR2YCrCb);
    int V = 0; // V => index 2
    
    /////////////////////////////////////////////////////////////////////////////
    // Image block processing : for each (block_size*block_size) block
    mark.clear();
    int i = 0, j = 0, message = -1;
    for (size_t index = 0; index < pos_block_col.size(); index++) {
        
        i = block_size * pos_block_row[index];
        j = block_size * pos_block_col[index];
        
        // Select a block
        Mat block = image_mark(Rect(j, i, block_size, block_size));
        
        // Split in channels
        vector<Mat> planes;
        split(block, planes);
        vector<Mat> dctplanes(planes.size()); // Planes for DCT transform
        
        /////////////////////////////////////////////////////////////////////////////
        // direct DCT
        planes[V].convertTo(planes[V], CV_32FC1);
        dct(planes[V], dctplanes[V]);
        
        /////////////////////////////////////////////////////////////////////////////
        // detection
        detect_block(dctplanes[V], message, key[index], alpha, delta);
        mark.push_back(message);
    }
}

//////MARK GENERATION AND COMPARISON////////////////////////////////////////////////////////////////////////////////////////////

/**
 * getContentMarkBinary
 *
 * generate a binary mark from the content of the image
 *
 * @param image
 * @param mark
 * @param dctblock_size
 */
void getContentMarkBinary(Mat& image, vector<int>& mark,
                          int dctblock_size) {
    if (!dctblock_size > 0) {
        cerr << "Size of block must be > 0 ." << endl;
        return;
    }
    
    mark.clear();
    float lastDctCoef(0);
    int coefBigger(0);
    
    /////////////////////////////////////////////////////////////////////////////
    // Subsampling
    Mat image_sub;
    resize(image, image_sub, Size(image.cols / 4, image.rows / 4));
    
    // RGB => HSV
    cvtColor(image_sub, image_sub, CV_BGR2YCrCb);
    int V = 1;
    
    // Split block in channels
    vector<Mat> planes;
    split(image_sub, planes);
    for (size_t t = 0; t < planes.size(); t++)
        planes[t].convertTo(planes[t], CV_32FC1);
    
    /////////////////////////////////////////////////////////////////////////////
    // Block processing
    for (size_t i = 0; i < image_sub.size().height - dctblock_size; i +=
         dctblock_size) {
        for (size_t j = 0; j < image_sub.size().width - dctblock_size; j +=
             dctblock_size) {
            
            Mat block;
            Mat dctblock;
            
            /////////////////////////////////////////////////////////////////////////////
            // Select the block on intensity channel (V)
            block = planes[V](Rect((int)j,(int) i, block_size, block_size));
            
            /////////////////////////////////////////////////////////////////////////////
            // Direct DCT
            dct(block, dctblock);
            
            /////////////////////////////////////////////////////////////////////////////
            //// BINARY MARK : comparison between first DCT coefficients of consecutive blocks (=mean)
            // Compare first DCT coefficients
            float currentDctCoef(dctblock.at<float>(0, 0));
            coefBigger = (currentDctCoef > lastDctCoef) ? 1 : 0;
            mark.push_back(coefBigger);
            lastDctCoef = currentDctCoef;
        }
    }
}

/**
 * compareMarks
 *
 * compare the given marks and return the number of good matches. Return negative value if the marks have different size.
 *
 * @param mark1
 * @param mark2
 */
int compareMarks(vector<int> mark1, vector<int> mark2) {
    int mmax = min((int)mark1.size(), (int)mark2.size());
    int goodmatches = 0;
    
    for (size_t i = 0; i < mmax; ++i)
        if (mark1[i] == mark2[i])
            goodmatches++;
    
    return ((mark1.size() == mark2.size()) ? 1 : -1) * goodmatches;
}

/**
 * displayMeasures
 *
 * Display in the terminal the similarity measures and the mark comparison between the two images and the two marks given.
 *
 * @param image1
 * @param mark1
 * @param image2
 * @param mark2
 */
void displayMeasures(Mat& image1, vector<int>& mark1, Mat& image2,
                     vector<int>& mark2) {
    double psnr = getPSNR(image1, image2);
    double ssim = getSSIM(image1, image2);
    int good_detection = compareMarks(mark1, mark2);
    
    if (good_detection < 0) {
        cout << " Marks have different sizes !" << endl;
        good_detection *= -1;
    }
    
    cout << " Similarity measures PSNR : " << psnr << " | SSIM : "
    << ssim << endl;
    cout << " Detection rate     : "
    << (double) 100 * good_detection / mark1.size() << "% ("
    << good_detection << "/" << mark1.size() << ")" << endl;
}

//////SAVE AND LOAD KEY FILES////////////////////////////////////////////////////////////////////////////////////////////////

//// read/writeAllKeyFile : for key files with multiple frames : marked video. Also works for a 1-frame video = image.
//// read/saveImageKeyFile : for 1 image, easier and faster

/**
 * writeAllKeyFile
 *
 * write the keyfile (framenums + allkeys + allrows + allcols) in the outstream.
 *
 * @param str
 * @param framenums
 * @param allkeys
 * @param allcols
 * @param allrows
 * @return
 */
bool writeAllKeyFile(ostream& str, vector<int> framenums,
                     vector<vector<double> > allkeys, vector<vector<int> > allcols,
                     vector<vector<int> > allrows) {
    
    // Check vectors size
    int totalframes = (int) framenums.size();
    if (allkeys.size() != totalframes || allrows.size() != totalframes
        || allcols.size() != totalframes) {
        return false;
    }
    
    writeNextRow(str, framenums);
    
    for (size_t index = 0; index < totalframes; index++) {
        writeNextRow(str, allkeys[index]);
        writeNextRow(str, allcols[index]);
        writeNextRow(str, allrows[index]);
    }
    
    return true;
}

/**
 * readAllKeyFile
 *
 * read the instream as a key file (framenums + allkeys + allrows + allcols).
 *
 * @param str
 * @param framenums
 * @param allkeys
 * @param allcols
 * @param allrows
 * @return
 */
bool readAllKeyFile(istream& str, vector<int> &framenums,
                    vector<vector<double> > &allkeys, vector<vector<int> > &allcols,
                    vector<vector<int> > &allrows) {
    
    // Init vectors
    framenums.clear();
    allkeys.clear();
    allrows.clear();
    allcols.clear();
    
    vector<int> cols, rows;
    vector<double> keys;
    
    // Read file row by row
    readNextRow(str, framenums);
    
    for (size_t index = 0; index < framenums.size(); index++) {
        if (!readNextRow(str, keys)) {
            return false;
        }
        allkeys.push_back(keys);
        if (!readNextRow(str, cols)) {
            return false;
        }
        allcols.push_back(cols);
        if (!readNextRow(str, rows)) {
            return false;
        }
        allrows.push_back(rows);
    }
    
    // Check vectors size
    int totalframes = (int) framenums.size();
    if (allkeys.size() != totalframes || allrows.size() != totalframes
        || allcols.size() != totalframes) {
        cout << "Vectors have dirrerent size" << endl;
    }
    return true;
}

/**
 * writeImageKeyFile
 *
 * write the key file of a single image (#frame = 0 + keys + #cols + #rows) in the outstream.
 *
 * @param ostr
 * @param keys
 * @param cols
 * @param rows
 * @return
 */
bool writeImageKeyFile(ostream& ostr, vector<double> keys,
                       vector<int> cols, vector<int> rows) {
    
    ostr << "0" << endl;
    writeNextRow(ostr, keys);
    writeNextRow(ostr, cols);
    writeNextRow(ostr, rows);
    
    return true;
}

/**
 * readImageKeyFile
 *
 * read the instream as a key file for a single image (#frame = 0 + keys + #cols + #rows).
 *
 * @param istr
 * @param keys
 * @param cols
 * @param rows
 * @return
 */
bool readImageKeyFile(istream& istr, vector<double>& keys,
                      vector<int>& cols, vector<int>& rows) {
    
    // Read first line to be sure that there is an image marked.
    vector<int> framenums;
    readNextRow(istr, framenums);
    if (framenums[0] != 0) {
        return false;
    }
    if (!readNextRow(istr, keys)) {
        return false;
    }
    if (!readNextRow(istr, cols)) {
        return false;
    }
    if (!readNextRow(istr, rows)) {
        return false;
    }
    
    return true;
}

//////OTHERS//////OTHERS//////OTHERS///////////////////////////////////////////////////////////////////////////////////////

/**
 * Quantizer used for insertion and detection
 * @param value
 * @param delta
 * @return
 */
double quantize(double value, double delta) {
    return delta * round(value / delta);
}

/**
 * Quantizer using default delta step size
 * @param value
 * @return
 */
double quantize(double value) {
    return quantize(value, delta_q);
}

/**
 * insert_block
 *
 * Insert the given message in the given block
 *
 * @param block
 * @param mark
 * @param key
 * @param alpha
 * @param delta
 */
void insert_block(Mat& block, int mark, double key, double alpha,
                  int delta) {
    
    double x = 0, w = 0, qtt = 0;
    
    //// PERCEPTION MASK ////
    Mat masque = Mat(4, 4, CV_32FC1, &masque_t);
    Mat normalized_masque = masque / norm_masque;
    
    //// PROJECITON ////
    Mat prod;
    multiply(block, normalized_masque, prod);
    x = sum(prod).val[0];
    
    //// MARK GENERATION ////
    qtt = x - delta * ((double) mark / 2 + key);
    w = alpha * (quantize(qtt, delta) - qtt);
    
    //// MARK INSERTION ////
    add(block, w * normalized_masque, block);
}

/**
 * detection_measure
 *
 * measure on the block used for the decision during the detection and the insertion using prior detection criteria.
 *
 * @param block
 * @param key
 * @param alpha
 * @param delta
 * @return
 */
double detection_measure(Mat& block, double key, double alpha,
                         int delta) {
    double x = 0, qtt = 0;
    //// PERCEPTION MASK ////
    Mat masque = Mat(4, 4, CV_32FC1, &masque_t);
    
    //// PROJECTION ////
    Mat prod;
    multiply(block, masque / norm_masque, prod);
    x = sum(prod).val[0];
    
    //// MARK DETECTION ////
    qtt = x - delta * key;
    return quantize(qtt, delta) - qtt;
}

/**
 * detect_block
 *
 * detect the message in the given block by decision on the detection measure
 *
 * @param block
 * @param mark
 * @param key
 * @param alpha
 * @param delta
 */
void detect_block(Mat& block, int& mark, double key, double alpha,
                  int delta) {
    //// MARK DETECTION ////
    double D = detection_measure(block, key, alpha, delta);
    
    //// DECISION ////
    mark = (fabs(D) <= (1 - alpha) * delta / 2) ? 0 : 1;
}

/**
 * readNextRow
 *
 * read the next row of the given instream as a vector where values are separated by separator (char)
 *
 * @param str
 * @param rowdata
 * @return
 */
template<class T>
bool readNextRow(istream& str, vector<T>& rowdata) {
    
    string line;
    stringstream lineStream;
    string cell;
    T value;
    
    // Read line as row is a stream
    if (getline(str, line)) {
        lineStream << line;
        
        // Init and fill
        rowdata.clear();
        while (getline(lineStream, cell, separator)) {
            stringstream converter(cell);
            converter >> value;
            rowdata.push_back(value);
        }
        return true;
    } else {
        // Cannot read a line
        return false;
    }
}

/**
 * writeNextRow
 *
 * write the given vector in the given outstream as a line where values are separated by separator (char)
 *
 * @param str
 * @param rowdata
 * @return
 */
template<class T>
bool writeNextRow(ostream& str, vector<T> rowdata) {
    
    typename vector<T>::const_iterator row_it;
    for (row_it = rowdata.begin(); row_it != rowdata.end(); ++row_it) {
        str << *row_it;
        str << separator;
    }
    str << endl;
    return true;
}
