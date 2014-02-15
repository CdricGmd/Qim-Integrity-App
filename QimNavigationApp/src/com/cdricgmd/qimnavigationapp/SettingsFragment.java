package com.cdricgmd.qimnavigationapp;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;

public class SettingsFragment extends PreferenceFragment implements
		OnSharedPreferenceChangeListener {
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
		initPreferenceSummaries();
	}

	public static final String KEY_PREF_MQIM_DELTA = "pref_key_mqim_delta";
	public static final String KEY_PREF_MQIM_SELECTIONMETHOD = "pref_key_mqim_selectionmethod";

	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
			String key) {
		if (key.equals(KEY_PREF_MQIM_DELTA)) {
			// EditTextPreference deltaPref = (EditTextPreference) findPreference(key);
			// Set summary to be the user-description for the selected value
			int d = Integer.parseInt(sharedPreferences.getString(key, "150"));
			WatermarkingEngine.setDelta(d);
		} else if (key.equals(KEY_PREF_MQIM_SELECTIONMETHOD)) {
			// ListPreference selectionmethodPref = (ListPreference) findPreference(key);
			// Set summary to be the user-description for the selected value
			int m = Integer.parseInt(sharedPreferences.getString(key, "0"));
			WatermarkingEngine.setSelectionMethodIndex(m);
		}
		initPreferenceSummaries();
	}

	@Override
	public void onResume() {
		super.onResume();
		getPreferenceManager().getSharedPreferences()
				.registerOnSharedPreferenceChangeListener(this);

	}

	@Override
	public void onPause() {
		getPreferenceManager().getSharedPreferences()
				.unregisterOnSharedPreferenceChangeListener(this);
		super.onPause();
	}

	public void initPreferenceSummaries() {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(getActivity()
						.getApplicationContext());
		//
		EditTextPreference deltaPref = (EditTextPreference) findPreference(KEY_PREF_MQIM_DELTA);
		deltaPref.setSummary("Delta: "
				+ sharedPreferences.getString(KEY_PREF_MQIM_DELTA, "150"));
		//
		ListPreference selectionmethodPref = (ListPreference) findPreference(KEY_PREF_MQIM_SELECTIONMETHOD);
		selectionmethodPref.setSummary("Criteria: "
				+ selectionmethodPref.getEntry());
		//

	}
}
