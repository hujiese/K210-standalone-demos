package com.scut.hujie.oio;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

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
}
