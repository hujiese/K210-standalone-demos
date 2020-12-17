package com.scut.hujie.nettyClient;

import com.scut.hujie.netty.K210Msg;
import com.scut.hujie.netty.Utils;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;

import java.util.List;

public class K210MsgDecoder extends ByteToMessageDecoder {

    void setMsgErr(List<Object> out){
        K210Msg msg = new K210Msg(1);
        msg.setMsgOk(false);
        out.add(msg);
    }

    @Override
    public void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        try {
            if(in.readableBytes() > 0){
                byte magicNum = in.readByte(); // 获取魔数
                // 魔数必须是0x66，而且后面有效荷载的长度要>=8字节
                if(magicNum == Utils.MAGIC_NUM && in.readableBytes() >= Utils.MSG_MIN_PAYLOAD_LEN){
                    byte direction = in.readByte();
                    byte is_detect_object = in.readByte();
                    byte is_detect_car = in.readByte();
                    byte is_detect_person = in.readByte();
                    int distance = in.readInt();
                    int imgLength = in.readableBytes();
                    K210Msg msg = new K210Msg(imgLength);
                    msg.setMsgOk(true);
                    msg.setDirection(direction);
                    msg.set_detect_object(is_detect_object == Utils.BYTE_TRUE_FLAG ? true : false);
                    msg.set_detect_car(is_detect_car == Utils.BYTE_TRUE_FLAG ? true : false);
                    msg.set_detect_person(is_detect_person == Utils.BYTE_TRUE_FLAG ? true : false);
                    msg.setDistance(distance);
                    ByteBuf imgBuf = in.readBytes(imgLength);
                    byte[] imgBytes = new byte[imgBuf.readableBytes()];
                    imgBuf.readBytes(imgBytes);
//                msg.copyImage(imgBytes, 0, imgLength); // 深拷贝
                    msg.setJpegImage(imgBytes); // 浅拷贝
                    out.add(msg);
                } else {
                    setMsgErr(out);
                }
            }else{
                setMsgErr(out);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}