package com.scut.hujie.k210nettyandroid.decoder;

import android.util.Log;

import com.scut.hujie.k210nettyandroid.bean.K210Msg;
import com.scut.hujie.k210nettyandroid.utils.Const;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;

import java.util.List;

public class K210MsgDecoder extends ByteToMessageDecoder {

//    void setMsgErr(List<Object> out){
//        K210Msg msg = new K210Msg(1);
//        msg.setMsgOk(false);
//        out.add(msg);
//    }

    @Override
    public void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        try {
            if(in.readableBytes() > 0){
                byte magicNum = in.readByte(); // 获取魔数
                // 魔数必须是0x66，而且后面有效荷载的长度要>=8字节
                if(magicNum == Const.MAGIC_NUM && in.readableBytes() >= Const.MSG_MIN_PAYLOAD_LEN){
                    byte retType = in.readByte();
                    switch (retType)
                    {
                        case Const.FORWARD_DIRECTION :
                        case Const.LEFT_DIRECTION :
                        case Const.RIGHT_DIRECTION :
                            // 封装目标检测返回消息
                            makeDetectMsg(in, out, retType);
                            break;
                        case Const.CMD_DISTANCE :
                            // 封装测距返回消息
                            makeDistanceMsg(in, out, retType);
                            break;
                        case Const.CMD_SERVO :
                            // 封装移动舵机返回消息
                            makeServoMsg(in, out, retType);
                            break;
                        default:
                            // 消息无法识别，报错
                            Log.e("ERROR:", "UNKONWN CMD");
//                            setMsgErr(out);
                            break;
                    }
                } else {
                    Log.e("ERROR:", "MSG broken");
//                    setMsgErr(out);
                }
            }else{
                Log.e("ERROR:", "MSG too small");
//                setMsgErr(out);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /*
    *  控制舵机回复消息格式
    *
    *  |--0-1-2-3-|--------4-------|---------------5-------------|-------6------|------7~12-----|-------other-------|
    *  | 消息长度 | 0x66(固定魔数) |      消息返回类型(0x06)      | 0x01(执行ok) | 0x00(固定值)  | 消息返回类型(0x06) |
    */
    private void makeServoMsg(ByteBuf in, List<Object> out, byte retType) {
        byte servoDoneFlag = in.readByte();
        byte[] emptyContent = new byte[6];
        in.readBytes(emptyContent); // 处理掉7~12字节的固定值0x00
        byte msgEndFlag = in.readByte();// 读取最后一个字节，这里必须是0x06，否则出错
        if(msgEndFlag == Const.CMD_SERVO){
            K210Msg msg = new K210Msg();
            msg.setMsgOk(true);
            msg.setRetMsgType(retType);
            msg.setServoMove(servoDoneFlag == Const.BYTE_TRUE_FLAG ? true : false);
            out.add(msg);
        }else{
//            setMsgErr(out);
            Log.e("ERROR:", "SERVO MSG broken");
        }
    }

    /*
    *  测距回复消息格式
    *
    *  |--0-1-2-3--|--------4-------|---------------5-------------|--------6~8-------|---9~12--|-------other--------|
    *  |  消息长度 | 0x66(固定魔数) |      消息返回类型(0x05)      |    0x00(固定值)  | 测距值  | 消息返回类型(0x05) |
    */
    private void makeDistanceMsg(ByteBuf in, List<Object> out, byte retType) {
        byte[] emptyContent = new byte[3];
        in.readBytes(emptyContent); // 处理掉6~8字节的固定值0x00
        int distance = in.readInt(); // 读取距离值
        byte msgEndByte = in.readByte(); // 读取最后一个字节，这里必须是0x05，否则出错
        if(msgEndByte == Const.CMD_DISTANCE){
            K210Msg msg = new K210Msg();
            msg.setMsgOk(true);
            msg.setRetMsgType(retType);
            msg.setDistance(distance);
            out.add(msg);
        }
        else{
//            setMsgErr(out);
            Log.e("ERROR:", "DISTANCE MSG broken");
        }

    }

    /*
    *  目标检测回复消息格式
    *
    *  |--0-1-2-3-|--------4-------|---------------5-------------|-------6------|-------7------|-----8-----|--9~12---|------other-----|
    *  | 消息长度 | 0x66(固定魔数) | 消息返回类型(0x20/0x21/0x22) | 检测目标标志 | 检测车辆标志 | 检测人标志 | 测距值  | 二进制图片内容 |
    */
    private void makeDetectMsg(ByteBuf in, List<Object> out, byte retType) {
        byte isDetectObject = in.readByte();
        byte isDetectCar = in.readByte();
        byte isDetectPerson = in.readByte();
        int distance = in.readInt();
        int imgLength = in.readableBytes();
        K210Msg msg = new K210Msg(imgLength);
        msg.setMsgOk(true);
        msg.setRetMsgType(retType);
        msg.setDetectObject(isDetectObject == Const.BYTE_TRUE_FLAG ? true : false);
        msg.setDetectCar(isDetectCar == Const.BYTE_TRUE_FLAG ? true : false);
        msg.setDetectPerson(isDetectPerson == Const.BYTE_TRUE_FLAG ? true : false);
        msg.setDistance(distance);
        ByteBuf imgBuf = in.readBytes(imgLength);
        byte[] imgBytes = new byte[imgBuf.readableBytes()];
        imgBuf.readBytes(imgBytes);
//        msg.copyImage(imgBytes, 0, imgLength); // 深拷贝
        msg.setJpegImage(imgBytes); // 浅拷贝
        out.add(msg);
    }
}