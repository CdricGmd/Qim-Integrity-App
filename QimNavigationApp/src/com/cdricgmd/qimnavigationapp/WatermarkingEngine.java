package com.cdricgmd.qimnavigationapp;

import java.io.FileOutputStream;

import org.opencv.android.Utils;
import org.opencv.core.Mat;

import android.graphics.Bitmap;
import android.util.Log;

public class WatermarkingEngine {

	private static final String TAG = "IntegrityApp::watermarkingEngine";

	/*
	 * Load native library
	 */
	/*
	 * static { try { System.loadLibrary("QimNativeEngine"); Log.i(TAG,
	 * "Native code library successfully loaded"); } catch (UnsatisfiedLinkError
	 * e) { Log.v(TAG, "Native code library failed to load.\n" + e); } catch
	 * (Exception e) { Log.v(TAG, "Native code library exception: " + e); } }
	 */

	/*
	 * Parameters
	 */
	private static int delta = 150;
	private static double alpha = 0.5;
	private static int selectionMethodIndex = 1;

	/*
	 * Native methods
	 */
	private native static void nativeInsertionQimRandom(long addrIm,
			String imDir, String keyDir, int delta, double alpha);

	private native static void nativeInsertionQimPriorDetectionCriteria(
			long addrIm, String imDir, String keyDir, int delta, double alpha);

	private native static void nativeInsertionQimPriorEnergyCriteria(
			long addrIm, String imDir, String keyDir, int delta, double alpha);

	private native static double nativeInsertionQimDetection(long addrIm,
			String keyDir, int delta, double alpha);

	private native static double nativeGetPSNR(long addrIm1, long addrIm2);

	private native static double nativeGetIF(long addrIm1, long addrIm2);

	private native static double nativeGetSSIM(long addrIm1, long addrIm2);

	private native static double nativeGetNCC(long addrIm1, long addrIm2);

	/*
	 * Public static methods for watermarking
	 */

	public static Bitmap insertion(Bitmap im) {
		Mat mat = new Mat();
		Utils.bitmapToMat(im, mat);
		// Log.i(TAG, "Insertion to : " + mat.cols() + " x " + mat.rows());
		// TODO

		switch (selectionMethodIndex) {
		case 0:
			// RANDOM
			nativeInsertionQimRandom(mat.getNativeObjAddr(),
					(String) (CommonResources.image_marked_path),
					(String) (CommonResources.key_saved_path), delta, alpha);
			break;
		case 1:
			// PRIOR DETECTION CRITERIA
			nativeInsertionQimPriorDetectionCriteria(mat.getNativeObjAddr(),
					(String) (CommonResources.image_marked_path),
					(String) (CommonResources.key_saved_path), delta, alpha);
			break;
		case 2:
			// PRIOR ENERGY CRITERIA
			nativeInsertionQimPriorEnergyCriteria(mat.getNativeObjAddr(),
					(String) (CommonResources.image_marked_path),
					(String) (CommonResources.key_saved_path), delta, alpha);
			break;
		default:
			Log.i(TAG, "No insertion method chosen");
			break;
		}

		// Result Mat to new Bitmap
		final Bitmap bmp = Bitmap.createBitmap((int) mat.size().width,
				(int) mat.size().height, Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(mat, bmp);

		// Save image
		new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					FileOutputStream out = new FileOutputStream(
							CommonResources.image_marked_path);
					if (bmp.compress(Bitmap.CompressFormat.PNG, 90, out))
						Log.i(TAG, "Save marked image at: "
								+ CommonResources.image_marked_path);
					out.close();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}).start();
		return bmp;
	}

	public static void detection() {
		/*
		 * if(CommonResources.image_marked == null ||
		 * CommonResources.key_saved_path.isEmpty()){ Log.i(TAG,
		 * "Cannot use detection : image or keyfile is missing") ; return; }
		 */

		Mat mat = new Mat();
		Utils.bitmapToMat(CommonResources.image, mat);

		Log.i(TAG, "Detection begin");
		CommonResources.check_integrityscore = nativeInsertionQimDetection(
				mat.getNativeObjAddr(), CommonResources.key_saved_path, delta,
				alpha);
		Log.i(TAG, "Detection done, score is "
				+ CommonResources.check_integrityscore);
	}

	/*
	 * Public static methods for measures
	 */

	public static double getPSNR(Bitmap bmp1, Bitmap bmp2) {
		Mat im1 = new Mat();
		Mat im2 = new Mat();
		Utils.bitmapToMat(bmp1, im1);
		Utils.bitmapToMat(bmp2, im2);
		return nativeGetPSNR(im1.getNativeObjAddr(), im2.getNativeObjAddr());
	}

	public static double getIF(Bitmap bmp1, Bitmap bmp2) {
		Mat im1 = new Mat();
		Mat im2 = new Mat();
		Utils.bitmapToMat(bmp1, im1);
		Utils.bitmapToMat(bmp2, im2);
		return nativeGetIF(im1.getNativeObjAddr(), im2.getNativeObjAddr());
	}

	public static double getNCC(Bitmap bmp1, Bitmap bmp2) {
		Mat im1 = new Mat();
		Mat im2 = new Mat();
		Utils.bitmapToMat(bmp1, im1);
		Utils.bitmapToMat(bmp2, im2);
		return nativeGetNCC(im1.getNativeObjAddr(), im2.getNativeObjAddr());
	}

	public static double getSSIM(Bitmap bmp1, Bitmap bmp2) {
		Mat im1 = new Mat();
		Mat im2 = new Mat();
		Utils.bitmapToMat(bmp1, im1);
		Utils.bitmapToMat(bmp2, im2);
		return nativeGetSSIM(im1.getNativeObjAddr(), im2.getNativeObjAddr());
	}

	/*
	 * Getters and setters
	 */

	public static int getDelta() {
		return delta;
	}

	public static void setDelta(int d) {
		delta = d;
	}

	public static void setSelectionMethodIndex(int index) {
		selectionMethodIndex = index;
	}

	public static int getSelectionMethodIndex() {
		return selectionMethodIndex;
	}

}
