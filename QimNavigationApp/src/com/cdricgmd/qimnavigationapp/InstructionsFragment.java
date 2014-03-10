package com.cdricgmd.qimnavigationapp;

import java.io.File;
import java.io.IOException;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.Toast;

public class InstructionsFragment extends Fragment {

	Uri picUri;
	private static final String TAG = "IntegrityApp::InstructionsFragment";

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {

		// VIEW: Inflate the layout for this fragment
		View V = inflater.inflate(R.layout.fragment_main_instructions,
				container, false);

		// BUTTONS LISTENERS
		ImageButton buttonLoadImage = (ImageButton) V
				.findViewById(R.id.buttonLoadImage);
		ImageButton buttonLoadCamera = (ImageButton) V
				.findViewById(R.id.buttonLoadCamera);
		ImageButton buttonLoadKey = (ImageButton) V
				.findViewById(R.id.buttonLoadKey);
		ImageButton buttonUnloadFiles = (ImageButton) V
				.findViewById(R.id.buttonUnloadFiles);

		buttonLoadImage.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				if (!CommonResources.isExternalStorageReadable()) {
					return;
				}

				Toast.makeText(getActivity().getApplicationContext(),
						getString(R.string.message_loadimage), Toast.LENGTH_SHORT).show();
				Intent i = new Intent(
						Intent.ACTION_PICK,
						android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
				startActivityForResult(i, CommonResources.RESULT_LOAD_IMAGE);
			}
		});

		buttonLoadCamera.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				if (!CommonResources.isExternalStorageReadable()) {
					return;
				}			

				// Create the File where the photo should go
				File imfiledir = CommonResources.getAlbumStorageDir(
						Environment.DIRECTORY_PICTURES, "IntegrityApp");
				CommonResources.mCapturedImage = imfiledir.getPath() + "/snapshot.jpg";
				File photo = new File(CommonResources.mCapturedImage);
				
				Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
			    // Ensure that there's a camera activity to handle the intent
			    if (takePictureIntent.resolveActivity(getActivity().getPackageManager()) != null) {
			        // Continue only if the File was successfully created
			        if (photo != null) {
			        	// Toast
						Toast.makeText(getActivity().getApplicationContext(),
								getString(R.string.message_takepicture), Toast.LENGTH_SHORT).show();
						
						// Start Activity
			            takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT,
			                    Uri.fromFile(photo));
			            startActivityForResult(takePictureIntent, CommonResources.RESULT_LOAD_CAMERA);
			        }
			    }
			}

		});

		buttonLoadKey.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				if (!CommonResources.isExternalStorageReadable()) {
					return;
				}
				Toast.makeText(getActivity().getApplicationContext(),
						getString(R.string.message_loadkey), Toast.LENGTH_SHORT).show();
				Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
				intent.setType("file/*");
				startActivityForResult(intent, CommonResources.RESULT_LOAD_KEY);
			}
		});

		buttonUnloadFiles.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				Toast.makeText(getActivity().getApplicationContext(),
						getString(R.string.message_unload), Toast.LENGTH_SHORT).show();
				CommonResources.unloadFiles();
				//Log.i(TAG, "Image and Key file unloaded.");

				// updateUI(getView());
				// Update all fragments
				CommonResources.updateStatus();
				for (Fragment frag : getActivity().getSupportFragmentManager()
						.getFragments()) {
					frag.onResume();
				}

				// Intent launchNewIntent = new Intent(getActivity(),
				// MainActivity.class);
				// startActivityForResult(launchNewIntent, RESULT_UNLOAD_FILES);
			}
		});
		return V;
	}

	@Override
	public void onResume() {
		super.onResume();
		updateUI(getView());
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {

		if (resultCode == Activity.RESULT_OK) {

			switch (requestCode) {
			case CommonResources.RESULT_LOAD_IMAGE:
				Uri selectedImage = data.getData();
				String[] filePathColumn = { MediaStore.Images.Media.DATA };
				Cursor cursor = getActivity().getContentResolver().query(
						selectedImage, filePathColumn, null, null, null);
				cursor.moveToFirst();
				int columnIndex = cursor.getColumnIndex(filePathColumn[0]);

				CommonResources.setImage_loaded_path(cursor
						.getString(columnIndex));
				cursor.close();

				//Log.i(TAG, "Image file loaded: "+ CommonResources.image_loaded_path);

				break;
			case CommonResources.RESULT_LOAD_KEY:
				CommonResources.key_saved_path = data.getData().getPath();

				//Log.i(TAG, "Key file loaded: " + CommonResources.key_saved_path);

				break;
			case CommonResources.RESULT_LOAD_CAMERA:
				galleryAddCameraPic();
				CommonResources.setImage_loaded_path(CommonResources.mCapturedImage);
				//Log.i(TAG, "Image file loaded from picture taken: "+ CommonResources.image_loaded_path);
				/*
				 * Bundle extras = data.getExtras(); // CommonResources.image =
				 * (Bitmap) extras.get("data"); Uri photo =
				 * extras.get(MediaStore.EXTRA_OUTPUT);
				 * 
				 * Log.i(TAG, "Camera image loaded: " +
				 * CommonResources.image_loaded_path);
				 */
				/*
				 * ContentValues values = new ContentValues();
				 * values.put(Images.Media.TITLE, "title");
				 * values.put(Images.Media.BUCKET_ID, "test");
				 * values.put(Images.Media.DESCRIPTION, "test Image taken");
				 * values.put(Images.Media.MIME_TYPE, "image/jpeg"); Uri uri =
				 * getContentResolver().insert(Media.EXTERNAL_CONTENT_URI,
				 * values);
				 */
				break;
			case CommonResources.RESULT_UNLOAD_FILES:
				break;
			default:
				break;
			}
			// updateUI(getView());
			// Update all fragments //
			// for(Fragment frag :
			// getActivity().getSupportFragmentManager().getFragments()){//
			// frag.onResume();//
			// }

		}
		CommonResources.updateStatus();
	}

	private void updateUI(View v) {
		updateButtons(v);
	}

	private void updateButtons(View v) {
		ImageButton buttonLoadImage = (ImageButton) v
				.findViewById(R.id.buttonLoadImage);
		ImageButton buttonLoadCamera = (ImageButton) v
				.findViewById(R.id.buttonLoadCamera);
		ImageButton buttonLoadKey = (ImageButton) v
				.findViewById(R.id.buttonLoadKey);
		// ImageButton buttonUnloadFiles = (ImageButton)
		// v.findViewById(R.id.buttonUnloadFiles);

		buttonLoadImage.setColorFilter(getResources().getColor(//
				("".equals(CommonResources.image_loaded_path)) ? //
				R.color.translucent_red
						: R.color.translucent_green));

		buttonLoadCamera.setColorFilter(getResources().getColor(//
				("".equals(CommonResources.image_loaded_path)) ? //
				R.color.translucent_red
						: R.color.translucent_green));

		buttonLoadKey.setColorFilter(getResources().getColor(//
				("".equals(CommonResources.key_saved_path)) ? //
				R.color.translucent_red
						: R.color.translucent_green));
	}
	
	private void galleryAddCameraPic() {
	    Intent mediaScanIntent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
	    File f = new File(CommonResources.mCapturedImage);
	    Uri contentUri = Uri.fromFile(f);
	    mediaScanIntent.setData(contentUri);
	    getActivity().sendBroadcast(mediaScanIntent);
	}
}