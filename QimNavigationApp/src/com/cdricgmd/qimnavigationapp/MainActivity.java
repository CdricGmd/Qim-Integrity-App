package com.cdricgmd.qimnavigationapp;

import java.util.Locale;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class MainActivity extends FragmentActivity {

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a
	 * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
	 * will keep every loaded fragment in memory. If this becomes too memory
	 * intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;

	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;

	private static final String TAG = "IntegrityApp::MarkActivity";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		// Create the adapter that will return a fragment for each of the
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(
				getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);

		// Load default values for the preferences
		// ONLY on first call (because third argument is set to false
		PreferenceManager.setDefaultValues(this, R.xml.preferences, false);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		Intent launchNewIntent;
		switch (item.getItemId()) {
		case R.id.action_settings:
			// SETTINGS
			launchNewIntent = new Intent(MainActivity.this,
					SettingsActivity.class);
			startActivityForResult(launchNewIntent, 0);
			return true;
		case R.id.action_share_image:
			if (CommonResources.areImageAndKeyAvailable()) {
				// SHARE IMAGE
				launchNewIntent = new Intent();
				launchNewIntent.setAction(Intent.ACTION_SEND);
				launchNewIntent.putExtra(Intent.EXTRA_STREAM,
						Uri.parse(CommonResources.image_marked_path));
				launchNewIntent.setType("image/*");
				startActivity(Intent.createChooser(launchNewIntent,
						getResources().getText(R.string.share_image_to)));
				// startActivityForResult(launchNewIntent, 0);
			} else {
				Toast.makeText(getApplicationContext(),
						getString(R.string.message_missing_image), Toast.LENGTH_SHORT)
						.show();
			}
			return true;
		case R.id.action_share_key:
			if (CommonResources.areImageAndKeyAvailable()) {
				// SHARE KEY
				launchNewIntent = new Intent();
				launchNewIntent.setAction(Intent.ACTION_SEND);
				launchNewIntent.putExtra(Intent.EXTRA_STREAM,
						Uri.parse(CommonResources.key_saved_path));
				// launchNewIntent.setType("file/*");
				launchNewIntent.setType("text/plain");
				startActivity(Intent.createChooser(launchNewIntent,
						getResources().getText(R.string.share_image_to)));
				// startActivityForResult(launchNewIntent, 0);
			} else {
				Toast.makeText(getApplicationContext(),
						getString(R.string.message_missing_key), Toast.LENGTH_SHORT)
						.show();
			}
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			Fragment fragment = null;

			// getItem is called to instantiate the fragment for the given page.
			switch (position) {
			case 1:
				// MARK
				fragment = new MarkFragment();
				break;
			case 2:
				// CHECK
				fragment = new CheckFragment();
				break;
			// break;
			default:
			case 0:
				// INSTRUCTIONS
				fragment = new InstructionsFragment();
				break;
			}

			// Bundle args = new Bundle();
			// args.putInt(MarkFragment.SOMETHING, VALUE);
			// fragment.setArguments(args);
			return fragment;

			// Return a DummySectionFragment (defined as a static inner class
			// below) with the page number as its lone argument.
			/*
			 * Fragment fragment = new DummySectionFragment(); Bundle args = new
			 * Bundle(); args.putInt(DummySectionFragment.ARG_SECTION_NUMBER,
			 * position + 1); fragment.setArguments(args); return fragment;
			 */
		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			Locale l = Locale.getDefault();
			switch (position) {
			case 0:
				return getString(R.string.title_section1).toUpperCase(l);
			case 1:
				return getString(R.string.title_section2).toUpperCase(l);
			case 2:
				return getString(R.string.title_section3).toUpperCase(l);
			}
			return null;
		}

	}

	// LOAD OpenCV application or ask for installing it.
	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				//Log.i(TAG, "OpenCV loaded successfully");
				try {
					System.loadLibrary("QimNativeEngine");
					//Log.i(TAG, "Native code library successfully loaded");
				} catch (UnsatisfiedLinkError e) {
					//Log.v(TAG, "Native code library failed to load.\n" + e);
				} catch (Exception e) {
					//Log.v(TAG, "Native code library exception: " + e);
				}
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	@Override
	public void onResume() {
		super.onResume();
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_6, this,
				mLoaderCallback);
		loadPrefs();
	}

	public void loadPrefs() {
		SharedPreferences sharedPref = PreferenceManager
				.getDefaultSharedPreferences(this);
		WatermarkingEngine.setDelta(Integer.parseInt(sharedPref.getString(
				SettingsFragment.KEY_PREF_MQIM_DELTA, "150")));
		WatermarkingEngine.setDelta(Integer.parseInt(sharedPref.getString(
				SettingsFragment.KEY_PREF_MQIM_SELECTIONMETHOD, "1")));
	}

}
