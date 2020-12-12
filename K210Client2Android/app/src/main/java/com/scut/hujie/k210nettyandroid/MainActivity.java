package com.scut.hujie.k210nettyandroid;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        new Thread(new Runnable() {
            Socket s = null;
            @Override
            public void run() {
                try {
                    s = new Socket("192.168.137.238", 8080);
                    while(true){
                        OutputStream os = s.getOutputStream();
                        os.write("hello !".getBytes());
                        Log.i(this.getClass().getName().toString(), "send ok ...");
                        try {
                            Thread.sleep(2000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

}
