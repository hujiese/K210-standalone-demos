package com.scut.hujie.k210nettyandroid.decoder;

import android.util.Log;

import com.scut.hujie.k210nettyandroid.bean.K210Msg;

import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;

//服务器端业务处理器
public class MsgHandler extends ChannelInboundHandlerAdapter {

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        try{
            K210Msg k210Msg = (K210Msg)msg;
            if(k210Msg.isMsgOk()) {
                Log.i("Handle Msg ok", msg.toString());
            } else {
                Log.i("Handle Msg fail", "receive msg fail !");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}