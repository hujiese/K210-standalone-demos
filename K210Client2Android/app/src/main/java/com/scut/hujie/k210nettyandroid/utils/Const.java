package com.scut.hujie.k210nettyandroid.utils;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class Const {

    /* 服务端相关配置 */
    public static final String ADDRESS = "192.168.137.234"; // 目标服务器IP地址
    public static final int PORT = 8080; // 目标服务器端口

    /* 消息相关标志 */
    public static final int RECV_BUFF_LENGTH = 30720; // 1024 * 30 接收数据包大小不超过30 kb
    public static final int MSG_MIN_PAYLOAD_LEN = 8; // 返回消息有效荷载最小长度
    public static final byte BYTE_TRUE_FLAG = 0x01; // 返回消息True 为0x01
    public static final byte BYTE_FALSE_FLAG = 0x00; // 返回消息False 为0x01
    public static final byte EMP_ARG = 0x00; // 指令空消息填充 为0x01
    public static final byte MAGIC_NUM = 0x66; // 魔数，标识K210返回的一个消息
    public static final byte CMD_END_FLAG = 0x0f; // 命令结束标志
    public static final byte CMD_DETECT = 0x01; // 检测命令
    public static final byte FORWARD_DIRECTION = 0x20; // 向前检测
    public static final byte LEFT_DIRECTION = 0x21; // 向左检测
    public static final byte RIGHT_DIRECTION = 0x22; // 向右检测
    public static final byte CMD_DISTANCE = 0x05; // 测距命令
    public static final byte CMD_SERVO = 0x06; // 舵机命令

    /* 测试命令(用于handler标识) */
    public static final byte DETECTLEFT = 0x00; // 检测左侧
    public static final byte DETECTFORWARD = 0x01; // 检测前方
    public static final byte DETECTRIGHT = 0x02; // 检测右侧
    public static final byte RECONNECT = 0x03; // 重连网络
    public static final byte DISTANCE = 0x04; // 测距指令
    public static final byte SERVOMOVE = 0x05; // 移动舵机
}
