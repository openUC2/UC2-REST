package com.api;

import android.util.Log;

import java.io.IOException;

import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;

public class ApiServiceCallbackAdapter<T> implements Callback<T> {

    private final ApiServiceCallback<T> callback;

    public ApiServiceCallbackAdapter(ApiServiceCallback<T> callback) {
        this.callback = callback;
    }

    public void onResponse(Call<T> call, Response<T> response) {
        if (response.isSuccessful()) {
            callback.onResponse(response.body());
        } else {
            if (response.code() == 504) {
                // HTTP 504 return code is used when the API successfully sent the message but not get a response within the timeout period.
                // It is important to NOT treat this as a failure; the execution status is UNKNOWN and could have been a success.
                return;
            }
            try {
                Log.e(ApiServiceCallbackAdapter.class.getSimpleName(), response.toString());
                RestError apiError = ApiServiceGenerator.getRestError(response);
                onFailure(call, new RestApiException(apiError));
            } catch (IOException e) {
                e.printStackTrace();
                onFailure(call, new RestApiException(e));
            }
        }
    }

    @Override
    public void onFailure(Call<T> call, Throwable throwable) {
        throwable.printStackTrace();
        if (throwable instanceof RestApiException) {
            callback.onFailure(throwable);
        } else {
            callback.onFailure(new RestApiException(throwable));
        }
    }
}
