package com.scut.hujie.k210nettyandroid;

import android.util.Log;

import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;

import java.util.logging.Logger;

//服务器端业务处理器
public class MsgHandler extends ChannelInboundHandlerAdapter {

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        try{
            K210Msg k210Msg = (K210Msg)msg;
            if(k210Msg.isMsgOk()) {
                Log.i("Handle Msg ok", msg.toString());
//                long startTime = System.currentTimeMillis();
//                Utils.nioCopyFile(Utils.IMG_FULL_PATH, k210Msg.getJpegImage().length, k210Msg.getJpegImage());
//                Utils.oioCopyFile(Utils.IMG_FULL_PATH, k210Msg.getJpegImage());
//                long endTime = System.currentTimeMillis();
//                logger.info("复制毫秒数：" + (endTime - startTime));
            }
            else{
                Log.i("Handle Msg ok", "receive msg fail !");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}