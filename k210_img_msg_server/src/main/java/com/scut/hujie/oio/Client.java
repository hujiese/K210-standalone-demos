package com.scut.hujie.oio;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;

// 用于测试k210作为服务端，该客户端建立长连接后和服务每过2s发送一次字符
public class Client {
    public static void main(String[] args) {
        Socket s = null;
        try {
            s = new Socket("192.168.137.238", 8080);
            while(true){
                OutputStream os = s.getOutputStream();
                os.write("hello !".getBytes());
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            try {
                s.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
