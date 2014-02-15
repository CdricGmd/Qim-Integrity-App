//
//  mQim.h
//  DCT image transform
//
//  Created by Cédric Golmard on 06/11/2013.
//

#ifndef __DCT_image_transform__mQim__
#define __DCT_image_transform__mQim__

#include <opencv2/opencv.hpp>
#include <string.h>

using namespace std;
using namespace cv;

/*
 * INSERTION
 */

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
		int delta);

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
		vector<int>& pos_block_row, double alpha, int delta);

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
		vector<int>& pos_block_row, double alpha, int delta);

/*
 * DETECTION
 */

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
		vector<int> pos_block_col, vector<int> pos_block_row, double alpha,
		int delta);

/*
 * MARKS GENERATION AND COMPARISON
 */

/**
 * getContentMarkBinary
 *
 * generate a binary mark from the content of the image
 *
 * @param image
 * @param mark
 * @param dctblock_size
 */
void getContentMarkBinary(Mat& image, vector<int>& mark, int dctblock_size);

/**
 * compareMarks
 *
 * compare the given marks and return the number of good matches. Return negative value if the marks have different size.
 *
 * @param mark1
 * @param mark2
 */
int compareMarks(vector<int> mark1, vector<int> mark2);

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
		vector<int>& mark2);

/*
 * SAVE AND LOAD KEYFILES
 */

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
bool writeAllKeyFile(ostream& str, vector<int> framenums, vector< vector<double> > allkeys, vector< vector<int> > allrows, vector< vector<int> > allcols);

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
bool readAllKeyFile(istream& str, vector<int> &framenums, vector< vector<double> > &allkeys, vector< vector<int> > &allrows, vector< vector<int> > &allcols);

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
bool writeImageKeyFile(ostream& ostr, vector<double> keys, vector<int> cols,
		vector<int> rows);

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
bool readImageKeyFile(istream& istr, vector<double>& keys, vector<int>& cols,
		vector<int>& rows);

#endif /* defined(__DCT_image_transform__mQim__) */
