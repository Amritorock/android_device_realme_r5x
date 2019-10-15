/*
 * Copyright (C) 2015 The CyanogenMod Project
 *               2017-2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.realmeparts.settings.kcal;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.DialogInterface;
import android.os.Bundle;

import com.realmeparts.settings.R;

public class PresetDialog extends DialogFragment {
    private CharSequence[] mEntries;
    private CharSequence[] mEntryValues;
    private String mValue;
    private KCalSettings mKCalSettingsFragment;
    private int mClickedDialogEntryIndex;
    private final DialogInterface.OnClickListener selectItemListener =
            new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (mClickedDialogEntryIndex != which) {
                        mValue = mEntryValues[which].toString();
                        mKCalSettingsFragment.applyValues(mValue);
                        mClickedDialogEntryIndex = which;
                    }
                    dialog.dismiss();
                }
            };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mEntries = getResources().getStringArray(R.array.preset_enteries);
        mEntryValues = getResources().getStringArray(R.array.preset_values);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder dialog = new AlertDialog.Builder(getActivity());
        dialog.setTitle(getString(R.string.presets_dialog_title));
        mClickedDialogEntryIndex = getValueIndex();
        dialog.setItems(mEntries, selectItemListener);
        return dialog.create();
    }

    private int getValueIndex() {
        return findIndexOfValue(mValue);
    }

    private int findIndexOfValue(String value) {
        if (value != null && mEntryValues != null) {
            for (int i = mEntryValues.length - 1; i >= 0; i--) {
                if (mEntryValues[i].equals(value)) {
                    return i;
                }
            }
        }
        return -1;
    }

    public void show(FragmentManager manager, String tag, KCalSettings mKCalSettingsFragment) {
        super.show(manager, tag);
        this.mKCalSettingsFragment = mKCalSettingsFragment;
    }
}
