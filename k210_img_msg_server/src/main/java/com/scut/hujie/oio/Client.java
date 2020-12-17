package com.scut.hujie.oio;

import java.io.*;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.Scanner;

// 用于测试k210作为服务端，该客户端建立长连接后通过读取终端命令给k210发送CMD
public class Client {

    public static void handleRecvImg(DataInputStream bis){
        try{
            // 4字节首部长度+1字节魔数标识
            if(bis.available() > 5){
                int msgLength = bis.readInt();
                if(msgLength <= Utils.RECV_BUFF_LENGTH){
                    System.out.println(msgLength);
                    byte[] msgPart = new byte[5]; // 获取识别信息
                    bis.read(msgPart);
                    if(msgPart[0] == Utils.MAGIC_NUM){
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
                            out.close();
                            System.out.println("write img ok...");
                        }catch (Exception e){
                            e.printStackTrace();
                        }
                    }
                }
            }
        }catch (IOException e){
            e.printStackTrace();
            try {
                bis.close();
            } catch (IOException e1) {
                e1.printStackTrace();
            }
        }
    }
    public static void main(String[] args) {
        Socket s = null;
        try {
            s = new Socket("192.168.137.238", 8080);
            Scanner scaner = new Scanner(System.in);
            while(true){
                OutputStream os = s.getOutputStream();
                DataInputStream bis = null;
                System.out.print("Please input your command: ");
                String cmd = scaner.nextLine().trim();
                // cmd msg: CMD 0x10 0x20 0x0f
                byte[] cmds = {'C', 'M', 'D', 0x10, 0x28, 0x0f };
                if(cmd.equals("L")){
                    cmds[4] = 0x21;
                }else if(cmd.equals("F")){
                    cmds[4] = 0x20;
                }else if(cmd.equals("R")){
                    cmds[4] = 0x22;
                }
                os.write(cmds);
                os.flush();
                bis = new DataInputStream (s.getInputStream());
                handleRecvImg(bis);
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
