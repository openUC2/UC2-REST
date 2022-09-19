package com.uc2control;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentTransaction;
import androidx.viewpager.widget.ViewPager;
import androidx.viewpager2.widget.ViewPager2;

import android.content.Context;
import android.os.Bundle;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TableLayout;

import com.google.android.material.tabs.TabLayout;
import com.google.android.material.tabs.TabLayoutMediator;
import com.uc2control.adapter.MainTabPageAdapter;

import dagger.hilt.android.AndroidEntryPoint;
import dagger.hilt.android.HiltAndroidApp;

@AndroidEntryPoint
public class MainActivity extends AppCompatActivity {

    private ViewPager2 pager;
    private MainTabPageAdapter pageAdapter;
    private TabLayout tabLayout;
    private TabLayoutMediator tabLayoutMediator;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        pageAdapter = new MainTabPageAdapter(this);
        pager = findViewById(R.id.pager);
        pager.setAdapter(pageAdapter);
        tabLayout = findViewById(R.id.tab_layout);
        tabLayoutMediator = new TabLayoutMediator(tabLayout, pager, new TabLayoutMediator.TabConfigurationStrategy() {
            @Override
            public void onConfigureTab(@NonNull TabLayout.Tab tab, int position) {
                switch(position)
                {
                    case 1:
                        tab.setText("Led");
                        break;
                    default:
                        tab.setText("Wifi");
                }
            }
        });
        tabLayoutMediator.attach();
    }

}