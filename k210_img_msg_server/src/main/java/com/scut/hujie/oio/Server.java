package com.scut.hujie.oio;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Arrays;

public class Server {
    public static void main(String[] args) {
        // 创建服务器Socket对象
        ServerSocket ss = null;
        try {
            ss = new ServerSocket(Utils.SOCKET_SERVER_PORT);
            while(!Thread.interrupted()){
                Socket s = null;
                DataInputStream  bis = null;
                try {
                    // 监听客户端连接
                    s = ss.accept();
                    while(true){
                        // 封装通道内流
                        try{
                            bis = new DataInputStream (s.getInputStream());
                        }catch (Exception e){
                            e.printStackTrace();
                            s.close();
                            break;
                        }
                        int msgLength = bis.readInt();
                        System.out.println(msgLength);
                        byte[] msgPart = new byte[5]; // 获取识别信息
                        bis.read(msgPart);
                        System.out.println(Arrays.toString(msgPart));
                        int distance = bis.readInt();
                        System.out.println(distance);
                        int imgLength = msgLength - msgPart.length - 4;
                        byte[] img = new byte[imgLength];
                        int length_tmp = imgLength;
                        int index = 0;// start from zero
                        int readSize = 0;
                        while ((readSize = bis.read(img, index, length_tmp)) != -1) {
                            length_tmp -= readSize;
                            if (length_tmp == 0) {
                                break;
                            }
                            index = index + readSize;
                        }
                        System.out.println(img.length);
                        try{
                            // 封装图片文件
                            FileOutputStream out = new FileOutputStream(Utils.IMG_FULL_PATH);
                            FileChannel fileChannel = out.getChannel();
                            fileChannel.write(ByteBuffer.wrap(img)); //将字节流写入文件中
                            fileChannel.force(true);//强制刷新
                            fileChannel.close();
                            System.out.println("write img ok...");
                        }catch (Exception e){
                            e.printStackTrace();
                        }
                    }
                } catch (Exception e){
                    bis.close();
                    s.close();
                    e.printStackTrace();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }


    }
}