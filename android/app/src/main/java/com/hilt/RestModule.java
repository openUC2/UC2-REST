package com.hilt;

import com.api.RestController;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;
import dagger.hilt.InstallIn;
import dagger.hilt.components.SingletonComponent;

@Module
@InstallIn(SingletonComponent.class)
public class RestModule {
    @Provides
    @Singleton
    public static RestController restController()
    {
        return new RestController();
    }
}
