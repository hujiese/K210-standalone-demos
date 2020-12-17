package com.scut.hujie.k210nettyandroid;

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
}
