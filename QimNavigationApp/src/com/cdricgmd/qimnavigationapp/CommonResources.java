package com.cdricgmd.qimnavigationapp;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Environment;
import android.util.Log;

public class CommonResources {
	private static final String TAG = "IntegrityApp::CommonResources";

	public static final int RESULT_LOAD_IMAGE = 1001;
	public static final int RESULT_LOAD_CAMERA = 1002;
	public static final int RESULT_LOAD_KEY = 1003;
	public static final int RESULT_UNLOAD_FILES = 1004;
	public static final int RESULT_MARKNOW = 1005;
	
	public static final String image_file_extension = ".png";
	public static final String key_file_extension = ".txt";
	public static String mCapturedImage;

	// Status to define the state of a processing
	// WAITING : initial value, wait for good conditions
	// READY : ready to process (conditions are OK)
	// PROCESSING : is computing...
	// DONE : processing done and successful
	// FAILURE : error while processing
	public enum Status {
		WAITING, READY, DONE, PROCESSING, FAILURE
	};

	public static Bitmap image = null;

	public static String image_loaded_path = "";
	public static String image_marked_path = "";
	public static String key_saved_path = "";

	public static Status mark_status = Status.WAITING;
	public static Status check_status = Status.WAITING;

	public static int check_time_ms = -1;
	public static int mark_time_ms = -1;
	public static double mark_psnr = -1;
	public static double mark_ssim = -1;
	public static double mark_ncc = -1;
	public static double mark_if = -1;
	public static double check_integrityscore = -1;

	public static void unloadFiles() {
		image = null;
		image_loaded_path = "";
		image_marked_path = "";
		key_saved_path = "";

		resetMeasures();
		resetScore();

		mark_status = Status.WAITING;
		check_status = Status.WAITING;
	}

	public static void resetMeasures() {
		mark_psnr = -1;
		mark_ssim = -1;
		mark_ncc = -1;
		mark_if = -1;
		mark_time_ms = -1;
	}

	public static void resetScore() {
		check_integrityscore = -1;
		check_time_ms = -1;
	}

	public static void updateStatus() {
		// mark_status
		switch (mark_status) {
		case WAITING:
			if (!"".equals(CommonResources.image_loaded_path))
				mark_status = Status.READY;
			break;
		case READY:
		case DONE:
		case PROCESSING:
		case FAILURE:
			if ("".equals(CommonResources.image_loaded_path))
				mark_status = Status.WAITING;
			break;
		default:
			break;
		}

		// check_status
		switch (check_status) {
		case WAITING:
			if (!"".equals(CommonResources.image_loaded_path)
					&& !"".equals(CommonResources.key_saved_path))
				check_status = Status.READY;
			break;
		case READY:
		case DONE:
		case PROCESSING:
		case FAILURE:
		default:
			if ("".equals(CommonResources.image_loaded_path)
					|| "".equals(CommonResources.key_saved_path))
				check_status = Status.WAITING;
			break;
		}
	}

	/* Checks if external storage is available for read and write */
	public boolean isExternalStorageWritable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			return true;
		}
		return false;
	}

	/* Checks if external storage is available to at least read */
	public static boolean isExternalStorageReadable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)
				|| Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
			return true;
		}
		return false;
	}

	// Environment.DIRECTORY_PICTURES
	public static File getAlbumStorageDir(String theenvironment,
			String albumName) {
		// Get the directory for the user's public pictures directory.
		File file = new File(
				Environment.getExternalStoragePublicDirectory(theenvironment),
				albumName);
		if (!file.mkdirs()) {
			Log.e(TAG, "Directory not created");
		}
		return file;
	}

	public static String getImage_loaded_path() {
		return image_loaded_path;
	}

	public static void setImage_loaded_path(String path) {
		image_loaded_path = path;
		image = BitmapFactory.decodeFile(image_loaded_path);
	}

	public static void setImage_marked_path(String path) {
		image_marked_path = path;
		image = BitmapFactory.decodeFile(image_marked_path);
	}

	public static boolean areImageAndKeyAvailable() {
		return (//
		(!"".equals(image_marked_path) && (new File(image_marked_path))
				.canRead())//
		&& //
		(!"".equals(key_saved_path) && (new File(key_saved_path)).canRead())//
		);
	}

	private static String getNewFileName() {
		/*
		Time now = new Time();
		now.setToNow();
		// String is: YMDTHMS
		return now.format2445 ();
		*/
		return new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
	}
	
	public static String getNewImageFileName(){
		return getNewFileName() + image_file_extension;
	}
	
	public static String getNewKeyFileName(){
		return getNewFileName() + key_file_extension;
	}
	
}
