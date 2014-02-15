#include <jni.h>
#include <android/log.h>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mQim.hpp"
#include "lsb.hpp"
#include "measurements.hpp"

using namespace std;
using namespace cv;

#define LOG_TAG "IntegrityApp::QimNativeEngine"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif

// WATERMARKING
JNIEXPORT void JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimRandom(
		JNIEnv *env, jobject obj, jlong addrIm, jstring imDir, jstring keyDir,
		jint delta, jdouble alpha);

JNIEXPORT void JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimPriorDetectionCriteria(
		JNIEnv *env, jobject obj, jlong addrIm, jstring imDir, jstring keyDir,
		jint delta, jdouble alpha);

JNIEXPORT void JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimPriorEnergyCriteria(
		JNIEnv *env, jobject obj, jlong addrIm, jstring imDir, jstring keyDir,
		jint delta, jdouble alpha);

JNIEXPORT jdouble JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimDetection(
		JNIEnv *env, jobject obj, jlong addrIm, jstring keyDir, jint delta,
		jdouble alpha);

// MEASURES
JNIEXPORT jdouble JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetPSNR(JNIEnv *env,
		jobject obj, jlong addrIm1, jlong addrIm2);

JNIEXPORT jdouble JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetIF(JNIEnv *env,
		jobject obj, jlong addrIm1, jlong addrIm2);

JNIEXPORT jdouble JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetNCC(JNIEnv *env,
		jobject obj, jlong addrIm1, jlong addrIm2);

JNIEXPORT jdouble JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetSSIM(JNIEnv *env,
		jobject obj, jlong addrIm1, jlong addrIm2);

#ifdef __cplusplus
}
#endif

typedef enum {
	RANDOM, DETECTION, ENERGY
} strategy;

void insertionChoosingMethod(Mat& image, const char* imDir, const char* keyDir,
		int delta, double alpha, strategy method);

JNIEXPORT void JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimRandom(
		JNIEnv *env, jobject obj, jlong addrIm, jstring imDir, jstring keyDir,
		jint delta, jdouble alpha) {

	Mat& image = *(Mat*) addrIm;

	const char* imfileDir = env->GetStringUTFChars(imDir, NULL);
	const char* keyfileDir = env->GetStringUTFChars(keyDir, NULL);

	insertionChoosingMethod(image, imfileDir, keyfileDir, delta, alpha, RANDOM);
}

JNIEXPORT void JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimPriorDetectionCriteria(
		JNIEnv *env, jobject obj, jlong addrIm, jstring imDir, jstring keyDir,
		jint delta, jdouble alpha) {

	Mat& image = *(Mat*) addrIm;

	const char* imfileDir = env->GetStringUTFChars(imDir, NULL);
	const char* keyfileDir = env->GetStringUTFChars(keyDir, NULL);

	insertionChoosingMethod(image, imfileDir, keyfileDir, delta, alpha,
			DETECTION);
}

JNIEXPORT void JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimPriorEnergyCriteria(
		JNIEnv *env, jobject obj, jlong addrIm, jstring imDir, jstring keyDir,
		jint delta, jdouble alpha) {

	Mat& image = *(Mat*) addrIm;

	const char* imfileDir = env->GetStringUTFChars(imDir, NULL);
	const char* keyfileDir = env->GetStringUTFChars(keyDir, NULL);

	insertionChoosingMethod(image, imfileDir, keyfileDir, delta, alpha, ENERGY);
}

void insertionChoosingMethod(Mat& image, const char* imDir, const char* keyDir,
		int delta, double alpha, strategy method) {

	vector<int> mark;
	vector<double> keys;
	vector<int> cols, rows;

	// Mark generation
	LOGD("Mark generation");
	getContentMarkBinary(image, mark, 8);

	// Insertion
	switch (method) {
	case RANDOM:
		LOGD("Insertion using random blocks");
		insertionRandom(image, mark, keys, cols, rows, alpha, delta);
		break;
	case DETECTION:
		LOGD("Insertion using prior detection criteria");
		insertionPriorDetectionCriteria(image, mark, keys, cols, rows, alpha,
				delta);
		break;
	case ENERGY:
		LOGD("Insertion using energy criteria");
		insertionPriorEnergyCriteria(image, mark, keys, cols, rows, alpha,
				delta);
		break;
	}

	// Save keys
	LOGD("Save key file");
	ofstream outstr(keyDir, std::ofstream::out);
	writeImageKeyFile(outstr, keys, cols, rows);
	outstr.close();

	// Removed : done in android
	// Save image
	// LOGD("Save marked image");
	// imwrite(imageDir, image);
}

JNIEXPORT jdouble JNICALL
Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeInsertionQimDetection(
		JNIEnv *env, jobject obj, jlong addrIm, jstring keyDir, jint delta,
		jdouble alpha) {

	Mat& image = *(Mat*) addrIm;

	const char* keyfileDir = env->GetStringUTFChars(keyDir, NULL);

	vector<int> mark_fromfile;
	vector<int> mark_fromcontent;
	vector<double> keys;
	vector<int> cols, rows;

	int goodmatches = 0, marksize = 1;

	// Read keys
	LOGD("Read key file");
	ifstream instr(keyfileDir);
	readImageKeyFile(instr, keys, cols, rows);
	instr.close();

	// Detect mark in image
	LOGD("Detect mark in image");
	detection(image, mark_fromfile, keys, cols, rows, alpha, delta);

	// Compute mark on image's content
	LOGD("Compute mark on image's content");
	getContentMarkBinary(image, mark_fromcontent, 8);

	// Compare and return
	LOGD("Compare marks and compute integrity score");
	goodmatches = compareMarks(mark_fromfile, mark_fromcontent);
	marksize = mark_fromfile.size();

	return (jdouble) goodmatches / marksize;
}

/*
 * MEASUREMENTS
 */

JNIEXPORT jdouble JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetPSNR(
		JNIEnv *env, jobject obj, jlong addrIm1, jlong addrIm2) {
	Mat& mIm1 = *(Mat*) addrIm1;
	Mat& mIm2 = *(Mat*) addrIm2;

	return (jdouble) getPSNR(mIm1, mIm2);
}
JNIEXPORT jdouble JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetIF(
		JNIEnv *env, jobject obj, jlong addrIm1, jlong addrIm2) {
	Mat& mIm1 = *(Mat*) addrIm1;
	Mat& mIm2 = *(Mat*) addrIm2;

	return (jdouble) getIF(mIm1, mIm2);
}
JNIEXPORT jdouble JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetNCC(
		JNIEnv *env, jobject obj, jlong addrIm1, jlong addrIm2) {
	Mat& mIm1 = *(Mat*) addrIm1;
	Mat& mIm2 = *(Mat*) addrIm2;

	return (jdouble) getNCC(mIm1, mIm2);
}
JNIEXPORT jdouble JNICALL Java_com_cdricgmd_qimnavigationapp_WatermarkingEngine_nativeGetSSIM(
		JNIEnv *env, jobject obj, jlong addrIm1, jlong addrIm2) {
	Mat& mIm1 = *(Mat*) addrIm1;
	Mat& mIm2 = *(Mat*) addrIm2;

	return (jdouble) getSSIM(mIm1, mIm2);
}
