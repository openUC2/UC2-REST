package com.api;

public class RestError {

        /**
         * Error code.
         */
        private int code;

        /**
         * Error message.
         */
        private String msg;

        public int getCode() {
            return code;
        }

        public void setCode(int code) {
            this.code = code;
        }

        public String getMsg() {
            return msg;
        }

        public void setMsg(String msg) {
            this.msg = msg;
        }
}
