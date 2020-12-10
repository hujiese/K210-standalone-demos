package com.scut.hujie.netty;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class Utils {

    public static final int SOCKET_SERVER_PORT = 18899;
    public static final String SOCKET_SERVER_IP = "0.0.0.0";
    public static final int RECV_BUFF_LENGTH = 30720; // 1024 * 30 接收数据包大小不超过30 kb
    public static final byte MAGIC_NUM = 0x66; // 魔数，标识K210返回的一个消息
    public static final byte BYTE_TRUE_FLAG = 0x01;
    public static final byte LEFT_DIRECTION = 0x02;
    public static final byte FORWARD_DIRECTION = 0x03;
    public static final byte RIGHT_DIRECTION = 0x04;

    public static final int MSG_MIN_PAYLOAD_LEN = 8;

    public static final String IMG_WRITE_PATH = "F:\\K210_ENV\\server_img_buf\\";
    public static final String IMG_NAME = "k210_netty.jpeg";
    public static final String IMG_FULL_PATH = IMG_WRITE_PATH + IMG_NAME;

    public static final String IMG_READ_PATH = "F:\\K210_ENV\\server_img_buf\\";
    public static final String READ_IMG_NAME = "src.jpeg";
    public static final String READ_IMG_FULL_PATH = IMG_READ_PATH + READ_IMG_NAME;

    // 写入图片耗时约50ms
    public static void nioCopyFile(String path, int length, byte[] img) {
        try {
            File file = new File(path);
            FileOutputStream out = null;
            FileChannel fileChannel = null;
            try {
                if(!file.exists()){
                    file.createNewFile();
                }
                out = new FileOutputStream(file);
                fileChannel = out.getChannel();
                ByteBuffer byteBuffer = ByteBuffer.allocate(length);
                byteBuffer.put(img);
                byteBuffer.flip();
                fileChannel.write(byteBuffer);
                byteBuffer.clear();
                fileChannel.force(true);
            } finally {
                fileChannel.close();
                out.close();
            }
        } catch (Exception e){
            e.printStackTrace();
        }
    }

    // 写入图片耗时约1ms
    public static void oioCopyFile(String path, byte[] img) {
        BufferedOutputStream bos = null;
        try {
            bos = new BufferedOutputStream(new FileOutputStream(path));
            try {
                bos.write(img);
                bos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
}
