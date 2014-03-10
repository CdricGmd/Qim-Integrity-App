package com.cdricgmd.qimnavigationapp;

import java.io.File;
import java.text.DecimalFormat;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.Fragment;
import android.text.Html;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class MarkFragment extends Fragment {

	private static final String TAG = "IntegrityApp::MarkFragment";

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {

		// VIEW: Inflate the layout for this fragment
		View v = inflater
				.inflate(R.layout.fragment_main_mark, container, false);

		// BUTTON LISTENER
		Button buttonMarkNow = (Button) v.findViewById(R.id.buttonMarkNow);
		buttonMarkNow.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {

				//Log.i(TAG, "Button: Mark");
				updateUI(getView());

				if (CommonResources.mark_status == CommonResources.Status.WAITING)
					Toast.makeText(getActivity().getApplicationContext(),
							getString(R.string.message_mark_load_image),
							Toast.LENGTH_SHORT).show();
				else if (CommonResources.mark_status == CommonResources.Status.PROCESSING) {
					Toast.makeText(
							getActivity().getApplicationContext(),
							getString(R.string.message_mark_already_processing),
							Toast.LENGTH_SHORT).show();
				} else if (CommonResources.check_status == CommonResources.Status.PROCESSING) {
					Toast.makeText(
							getActivity().getApplicationContext(),
							getString(R.string.message_check_already_processing),
							Toast.LENGTH_SHORT).show();
				} else {
					// Intent launchNewIntent = new
					// Intent(getActivity(),MainActivity.class);
					// startActivityForResult(launchNewIntent,
					// CommonResources.RESULT_MARKNOW);
					new InsertionTask().execute();
				}
			}
		});

		return v;
	}

	@Override
	public void onResume() {
		super.onResume();
		updateUI(getView());
	}

	/*
	 * @Override public void onActivityResult(int requestCode, int resultCode,
	 * Intent data) {
	 * 
	 * if (resultCode == Activity.RESULT_OK) {
	 * 
	 * switch (requestCode) { case CommonResources.RESULT_MARKNOW: // DO MARKING
	 * new InsertionTask().execute(); updateUI(getView()); break; default:
	 * break; } } }
	 */

	/**
	 * UI UPDATERS
	 */

	private void updateUI(View v) {
		CommonResources.updateStatus();
		try {
			// updateButtons(v);
			updateTextStatus(v);
			updateTextTime(v);
			updateTextMeasures(v);
			updateImageView(v);
		} catch (Exception e) {
			//Log.e(TAG, "Cannot update UI. "+e.getMessage());
		}
	}

	private void updateImageView(View v) {
		ImageView imageView = (ImageView) v
				.findViewById(R.id.imageview_image_loaded);

		imageView.clearColorFilter();

		switch (CommonResources.mark_status) {
		case DONE:
		case READY:
			imageView.setImageBitmap(CommonResources.image);
			break;
		case PROCESSING:
			imageView.setImageResource(R.drawable.work_grey);
			break;

		case FAILURE:
			imageView.setColorFilter(getResources().getColor(
					R.color.translucent_red));
			break;

		case WAITING:
		default:
			imageView.setImageResource(R.drawable.frame_grey);
			break;
		}
	}

	private void updateButtons(View v) {
		// Button buttonMarkNow = (Button) v.findViewById(R.id.buttonMarkNow);
		/*
		 * switch (CommonResources.mark_status) { case DONE:
		 * buttonMarkNow.setColorFilter(getResources().getColor(
		 * R.color.translucent_green)); break; case PROCESSING:
		 * buttonMarkNow.setColorFilter(getResources().getColor(
		 * R.color.translucent_blue)); break; case WAITING:
		 * buttonMarkNow.setColorFilter(getResources().getColor(
		 * R.color.translucent_red)); break; case READY:
		 * buttonMarkNow.setColorFilter(getResources().getColor(
		 * R.color.translucent_green)); break; case FAILURE:
		 * buttonMarkNow.setColorFilter(getResources().getColor(
		 * R.color.translucent_red)); break;
		 * 
		 * default: buttonMarkNow.clearColorFilter(); break; }
		 */
	}

	private void updateTextStatus(View v) {
		TextView statsView = (TextView) v
				.findViewById(R.id.textview_mark_statistics_status);

		switch (CommonResources.mark_status) {
		case DONE:
			statsView.setText(Html.fromHtml("<i>"+getString(R.string.marking_done)+"</i>"));
			break;
		case PROCESSING:
			statsView.setText(Html.fromHtml("<i>"+getString(R.string.marking_inprogress)+"</i>"));
			break;
		case WAITING:
			statsView.setText(Html
					.fromHtml("<i>"+getString(R.string.marking_waiting)+"</i>"));
			break;
		case READY:
			statsView.setText(Html.fromHtml("<i>"+getString(R.string.marking_ready)+"</i>"));
			break;
		case FAILURE:
			statsView.setText(Html.fromHtml("<i>"+getString(R.string.marking_fail)+"</i>"));
			break;

		default:
			break;
		}
	}

	private void updateTextMeasures(View v) {
		TextView statsView = (TextView) v
				.findViewById(R.id.textview_mark_statistics_measures);
		switch (CommonResources.mark_status) {
		case DONE:
			DecimalFormat df = new DecimalFormat("00.0000");
			statsView.setText(Html.fromHtml("" //
					+ ((CommonResources.mark_psnr >= 0) ? //
					"<b>PSNR: </b>" + df.format(CommonResources.mark_psnr)
							+ "<br>"
							: "")//
					+ ((CommonResources.mark_psnr >= 0) ? //
					"<b>SSIM: </b>" + df.format(CommonResources.mark_ssim)
							+ "<br>"
							: "") //
					+ ((CommonResources.mark_ncc >= 0) ? //
					"<b>NCC: </b>" + df.format(CommonResources.mark_ncc)
							+ "<br>"
							: "") //
					+ ((CommonResources.mark_if >= 0) ? //
					"<b>IF: </b>" + df.format(CommonResources.mark_if) + "<br>"
							: "") //
			));
			break;
		default:
			statsView.setText("");
			break;
		}
	}

	private void updateTextTime(View v) {
		TextView timeView = (TextView) v
				.findViewById(R.id.textview_mark_statistics_time);
		switch (CommonResources.mark_status) {
		case DONE:
			timeView.setText(Html.fromHtml("<b>"+ getString(R.string.checking_time) +"</b>"
					+ CommonResources.mark_time_ms + "ms"));
			break;
		default:
			timeView.setText("");
			break;
		}
	}

	/**
	 * ASYNC TASKS
	 */

	/**
	 *
	 */
	private class InsertionTask extends AsyncTask<Void, Long, Void> {
		/**
		 * The system calls this to perform work in a worker thread and delivers
		 * it the parameters given to AsyncTask.execute()
		 */
		protected Void doInBackground(Void... none) {

			// Reset
			CommonResources.mark_status = CommonResources.Status.PROCESSING;
			CommonResources.resetMeasures();
			publishProgress((long) -1.0);

			// Get file directories
			File imfiledir = CommonResources.getAlbumStorageDir(
					Environment.DIRECTORY_PICTURES, "IntegrityApp");
			File keyfiledir = CommonResources.getAlbumStorageDir(
					Environment.DIRECTORY_DOWNLOADS, "IntegrityApp");
			// Log.i(TAG, imfiledir.toString() + " | " + keyfiledir.toString());

			// Generate new file paths for image and key
			CommonResources.image_marked_path = imfiledir.toString() + "/"
					+ CommonResources.getNewImageFileName();
			CommonResources.key_saved_path = keyfiledir.toString() + "/"
					+ CommonResources.getNewKeyFileName();
			//Log.i(TAG, CommonResources.image_marked_path + " | " + CommonResources.key_saved_path);

			// Insertion
			long startTime = System.currentTimeMillis();
			CommonResources.image = WatermarkingEngine
					.insertion(CommonResources.image);
			CommonResources.mark_status = CommonResources.Status.DONE;
			publishProgress(System.currentTimeMillis() - startTime);

			// Measures
			new MeasuresTask().execute();
			galleryAddPic();
			return null;
		}

		protected void onProgressUpdate(Long... progress) {
			if (progress[0] > 0) {
				CommonResources.mark_time_ms = progress[0].intValue();
			}
			updateUI(getView());
		}

		/**
		 * The system calls this to perform work in the UI thread and delivers
		 * the result from doInBackground()
		 */
		protected void onPostExecute(Void result) {
			// Toast
			String toasttext = "";
			switch (CommonResources.mark_status) {
			case DONE:
				toasttext = getString(R.string.message_mark_success);
				break;
			case PROCESSING:
				toasttext = getString(R.string.message_mark_progress);
				break;
			case WAITING:
				toasttext = getString(R.string.message_missing_image);
				break;
			case READY:
				toasttext = getString(R.string.message_mark_ready);
				break;
			case FAILURE:
				toasttext = getString(R.string.message_mark_failed);
				break;

			default:
				break;
			}
			// TOAST
			Toast.makeText(getActivity().getApplicationContext(), toasttext,
					Toast.LENGTH_SHORT).show();
			// Update UI
			updateUI(getView());
		}
	}

	/**
	 * 
	 */
	public class MeasuresTask extends AsyncTask<Void, Void, Void> {
		/**
		 * The system calls this to perform work in a worker thread and delivers
		 * it the parameters given to AsyncTask.execute()
		 */
		protected Void doInBackground(Void... none) {
			// Compute measures
			Bitmap original = BitmapFactory
					.decodeFile(CommonResources.image_loaded_path);
			try{
				CommonResources.mark_psnr = WatermarkingEngine.getPSNR(original,
						CommonResources.image);
				CommonResources.mark_ssim = WatermarkingEngine.getSSIM(original,
						CommonResources.image);
			}
			catch(Exception e){
				//Log.e(TAG, e.getMessage());
			}
			
			// CommonResources.mark_if =
			// WatermarkingEngine.getIF(original,
			// CommonResources.image);
			// CommonResources.mark_ncc =
			// WatermarkingEngine.getNCC(original,
			// CommonResources.image);
			return null;
		}

		/**
		 * The system calls this to perform work in the UI thread and delivers
		 * the result from doInBackground()
		 */
		protected void onPostExecute(Void none) {
			updateUI(getView());
		}
	}

	public void onLowMemory() {
		super.onLowMemory();

		Toast.makeText(getActivity().getApplicationContext(),
				"Low memory /!\\", Toast.LENGTH_SHORT).show();
	}
	
	private void galleryAddPic() {
	    Intent mediaScanIntent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
	    File f = new File(CommonResources.image_marked_path);
	    Uri contentUri = Uri.fromFile(f);
	    mediaScanIntent.setData(contentUri);
	    getActivity().sendBroadcast(mediaScanIntent);
	}
}
