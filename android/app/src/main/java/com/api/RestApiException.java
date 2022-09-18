package com.api;

public class RestApiException extends RuntimeException {

    private RestError error;

    public RestApiException(RestError error) {
        this.error = error;
    }


    public RestApiException() {
        super();
    }

    public RestApiException(String message) {
        super(message);
    }

    public RestApiException(Throwable cause) {
        super(cause);
    }

    public RestApiException(String message, Throwable cause) {
        super(message, cause);
    }

    public RestError getError() {
        return error;
    }

    @Override
    public String getMessage() {
        if (error != null) {
            return error.getMsg();
        }
        return super.getMessage();
    }
}
