package com.cdricgmd.qimnavigationapp;

import java.text.DecimalFormat;

import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
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

public class CheckFragment extends Fragment {

	private static final String TAG = "IntegrityApp::CheckFragment";

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {

		// VIEW: Inflate the layout for this fragment
		View v = inflater.inflate(R.layout.fragment_main_check, container,
				false);

		// BUTTON LISTENER
		Button buttonCheckNow = (Button) v.findViewById(R.id.buttonCheckNow);
		buttonCheckNow.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {

				Log.i(TAG, "Button: Check");
				updateUI(getView());

				if ("".equals(CommonResources.image_marked_path))
					CommonResources
							.setImage_marked_path(CommonResources.image_loaded_path);

				if (CommonResources.check_status == CommonResources.Status.WAITING)
					Toast.makeText(getActivity().getApplicationContext(),
							getString(R.string.message_check_load_imagekey),
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
					new DetectionTask().execute();
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

	/**
	 * UI UPDATERS
	 */

	private void updateUI(View v) {
		CommonResources.updateStatus();
		try {
			updateTextStatus(v);
			updateTextTime(v);
			updateScore(v);
			updateImageView(v);
		} catch (Exception e) {
			Log.e(TAG, "Cannot update UI. " + e.getMessage());
		}
	}

	private void updateImageView(View v) {
		ImageView imageView = (ImageView) v
				.findViewById(R.id.imageview_image_loaded);

		imageView.clearColorFilter();

		switch (CommonResources.check_status) {
		case DONE:
			imageView.setImageBitmap(CommonResources.image);
			break;
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

	private void updateTextStatus(View v) {
		TextView statsView = (TextView) v
				.findViewById(R.id.textview_check_statistics_status);

		switch (CommonResources.check_status) {
		case DONE:
			statsView.setText(Html.fromHtml("<i>Checking done.</i>"));
			break;
		case PROCESSING:
			statsView.setText(Html.fromHtml("<i>Checking in progress.</i>"));
			break;
		case WAITING:
			statsView.setText(Html
					.fromHtml("<i>Waiting for an image and a key.</i>"));
			break;
		case READY:
			statsView.setText(Html.fromHtml("<i>Ready to check.</i>"));
			break;
		case FAILURE:
			statsView.setText(Html.fromHtml("<i>Error : interruption.</i>"));
			break;

		default:
			break;
		}
	}

	private void updateScore(View v) {
		// TEXT
		TextView statsView = (TextView) v
				.findViewById(R.id.textview_check_statistics_measures);
		switch (CommonResources.check_status) {
		case DONE:
			DecimalFormat df = new DecimalFormat("0.0000");
			String text = "";
			if (CommonResources.check_integrityscore >= 0) {
				text += "<b>Integrity score: </b>"//
						+ df.format(CommonResources.check_integrityscore); //
			}
			statsView.setText(Html.fromHtml(text));
			break;
		default:
			statsView.setText("");
			break;
		}

		// STARS RATING
		// Colors
		int colorTint = 0;
		if (CommonResources.check_status != CommonResources.Status.DONE) {
			colorTint = Color.DKGRAY;
		} else if (CommonResources.check_integrityscore > 0.95) {
			colorTint = getResources().getColor(R.color.translucent_green);
		} else if (CommonResources.check_integrityscore > 0.85) {
			colorTint = getResources().getColor(R.color.orange);
		} else {
			colorTint = getResources().getColor(R.color.translucent_red);
		}

		// Fill the stars
		ImageView imv;
		imv = (ImageView) v.findViewById(R.id.imageview_score1);
		ImageFromScore(imv, CommonResources.check_integrityscore, 0.80, 0.82,
				colorTint);

		imv = (ImageView) v.findViewById(R.id.imageview_score2);
		ImageFromScore(imv, CommonResources.check_integrityscore, 0.84, 0.86,
				colorTint);

		imv = (ImageView) v.findViewById(R.id.imageview_score3);
		ImageFromScore(imv, CommonResources.check_integrityscore, 0.88, 0.90,
				colorTint);

		imv = (ImageView) v.findViewById(R.id.imageview_score4);
		ImageFromScore(imv, CommonResources.check_integrityscore, 0.92, 0.94,
				colorTint);

		imv = (ImageView) v.findViewById(R.id.imageview_score5);
		ImageFromScore(imv, CommonResources.check_integrityscore, 0.96, 0.98,
				colorTint);

	}

	private void ImageFromScore(ImageView imv, double score, double min_score,
			double mid_score, int colorTint) {
		int d;
		if (score < min_score)
			d = R.drawable.ic_action_not_important;
		else if (score < mid_score)
			d = R.drawable.ic_action_half_important;
		else
			d = R.drawable.ic_action_important;

		imv.setImageResource(d);
		imv.setColorFilter(colorTint);
	}

	private void updateTextTime(View v) {
		TextView timeView = (TextView) v
				.findViewById(R.id.textview_check_statistics_time);
		switch (CommonResources.check_status) {
		case DONE:
			timeView.setText(Html.fromHtml("<b>Time: </b>"
					+ CommonResources.check_time_ms + "ms"));
			break;
		default:
			timeView.setText("");
			break;
		}
	}

	/**
	 * DETECTION
	 */
	private class DetectionTask extends AsyncTask<Void, Long, Void> {
		/**
		 * The system calls this to perform work in a worker thread and delivers
		 * it the parameters given to AsyncTask.execute()
		 */
		protected Void doInBackground(Void... none) {

			// Reset
			CommonResources.check_status = CommonResources.Status.PROCESSING;
			CommonResources.resetScore();
			publishProgress((long) -1.0);

			// Detection
			long startTime = System.currentTimeMillis();
			WatermarkingEngine.detection();
			CommonResources.check_status = CommonResources.Status.DONE;
			publishProgress(System.currentTimeMillis() - startTime);

			return null;
		}

		protected void onProgressUpdate(Long... progress) {
			if (progress[0] > 0) {
				CommonResources.check_time_ms = progress[0].intValue();
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
			switch (CommonResources.check_status) {
			case DONE:
				toasttext = getString(R.string.message_check_success);
				break;
			case PROCESSING:
				toasttext = getString(R.string.message_check_progress);
				break;
			case WAITING:
				if ("".equals(CommonResources.image_marked_path))
					toasttext += getString(R.string.message_missing_image)
							+ " ";
				if ("".equals(CommonResources.key_saved_path))
					toasttext += getString(R.string.message_missing_image);
				break;
			case READY:
				toasttext = getString(R.string.message_check_ready);
				break;
			case FAILURE:
				toasttext = getString(R.string.message_check_failed);
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

}
