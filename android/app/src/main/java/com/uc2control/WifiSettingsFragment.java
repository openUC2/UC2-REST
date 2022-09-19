package com.uc2control;

import android.os.Bundle;

import androidx.databinding.DataBindingUtil;
import androidx.databinding.Observable;
import androidx.databinding.adapters.TextViewBindingAdapter;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.uc2control.databinding.FragmentWifiSettingsBinding;

import dagger.hilt.android.AndroidEntryPoint;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link WifiSettingsFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
@AndroidEntryPoint
public class WifiSettingsFragment extends Fragment {

    private final String TAG = WifiSettingsFragment.class.getSimpleName();
    // TODO: Rename parameter arguments, choose names that match
    // the fragment initialization parameters, e.g. ARG_ITEM_NUMBER
    private static final String ARG_PARAM1 = "param1";
    private static final String ARG_PARAM2 = "param2";

    // TODO: Rename and change types of parameters
    private String mParam1;
    private String mParam2;
    private WifiSettingsModelView wifiSettingsModelView;
    private FragmentWifiSettingsBinding wifiSettingsFragmentBinding;
    private ArrayAdapter<String> adapter;

    public WifiSettingsFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @param param1 Parameter 1.
     * @param param2 Parameter 2.
     * @return A new instance of fragment WifiSettingsFragment.
     */
    // TODO: Rename and change types and number of parameters
    public static WifiSettingsFragment newInstance(String param1, String param2) {
        WifiSettingsFragment fragment = new WifiSettingsFragment();
        if (param1 != null && param2 != null) {
            Bundle args = new Bundle();
            args.putString(ARG_PARAM1, param1);
            args.putString(ARG_PARAM2, param2);
            fragment.setArguments(args);
        }
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (getArguments() != null) {
            mParam1 = getArguments().getString(ARG_PARAM1);
            mParam2 = getArguments().getString(ARG_PARAM2);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        wifiSettingsModelView = new ViewModelProvider(this).get(WifiSettingsModelView.class);
        wifiSettingsFragmentBinding =  DataBindingUtil.inflate(inflater, R.layout.fragment_wifi_settings, container, false);
        wifiSettingsFragmentBinding.setWifimodel(wifiSettingsModelView.getWifiSettingsModel());
        adapter = new ArrayAdapter(getContext(), android.R.layout.simple_list_item_1, wifiSettingsModelView.getWifiSettingsModel().getWifi_ssids());
        wifiSettingsModelView.getWifiSettingsModel().addOnPropertyChangedCallback(new Observable.OnPropertyChangedCallback() {
            @Override
            public void onPropertyChanged(Observable sender, int propertyId) {
                if (propertyId == BR.wifi_ssids)
                {
                    adapter = new ArrayAdapter(getContext(), android.R.layout.simple_list_item_1, wifiSettingsModelView.getWifiSettingsModel().getWifi_ssids());
                    wifiSettingsFragmentBinding.listviewWifissids.setAdapter(adapter);
                }
            }
        });
        wifiSettingsFragmentBinding.listviewWifissids.setAdapter(adapter);

        wifiSettingsFragmentBinding.listviewWifissids.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                if (view instanceof TextView)
                    wifiSettingsModelView.getWifiSettingsModel().setSsid((String) ((TextView) view).getText());
            }
        });
        return wifiSettingsFragmentBinding.getRoot();
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG,"onResume");
    }
}