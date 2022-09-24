package com.api;


import android.util.Log;

import java.io.IOException;
import java.lang.annotation.Annotation;
import java.util.concurrent.TimeUnit;

import okhttp3.Dispatcher;
import okhttp3.OkHttpClient;
import okhttp3.ResponseBody;
import retrofit2.Call;
import retrofit2.Converter;
import retrofit2.Response;
import retrofit2.Retrofit;
import retrofit2.converter.jackson.JacksonConverterFactory;

public class ApiServiceGenerator {
    private static final OkHttpClient sharedClient;
    private static  final Converter.Factory converterFactory = JacksonConverterFactory.create();

    static {
        Dispatcher dispatcher = new Dispatcher();
        dispatcher.setMaxRequestsPerHost(2);
        dispatcher.setMaxRequests(2);
        sharedClient = new OkHttpClient.Builder()
                .dispatcher(dispatcher)
                .pingInterval(20, TimeUnit.SECONDS)
                .build();
    }

    public ApiServiceGenerator()
    {
    }

    public static <S> S createService(Class<S> serviceClass, String apiUrl) {
        Retrofit.Builder retrofitBuilder = new Retrofit.Builder()
                .baseUrl(apiUrl)
                .addConverterFactory(converterFactory);
        retrofitBuilder.client(sharedClient);
        Retrofit retrofit = retrofitBuilder.build();
        return retrofit.create(serviceClass);
    }

    public static <T> T executeSync(Call<T> call) {
        try {
            Response<T> response = call.execute();
            if (response.isSuccessful()) {
                return response.body();
            } else {
                Log.e(ApiServiceGenerator.class.getSimpleName(), response.toString());
                RestError err =  getRestError(response);
                throw new RestApiException(err);
            }
        } catch (IOException e) {
            e.printStackTrace();
            throw new RestApiException(e);
        }
    }

    public static RestError getRestError(Response<?> response) throws IOException {
        return errorBodyConverter.convert(response.errorBody());
    }

    private static final Converter<ResponseBody, RestError> errorBodyConverter =
            (Converter<ResponseBody, RestError>) converterFactory.responseBodyConverter(
                    RestError.class, new Annotation[0], null);

}
