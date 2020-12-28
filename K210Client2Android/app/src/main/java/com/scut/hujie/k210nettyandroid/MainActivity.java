package com.scut.hujie.k210nettyandroid;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.util.Scanner;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;


public class MainActivity extends AppCompatActivity {

    private Button leftBtn = null;
    private Button forwardBtn = null;
    private Button rightBtn = null;

    Channel channel;

    public int port = 8080;
    public String address = "192.168.137.234";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();

    }

    private void initView() {

        connect();
        leftBtn = (Button) this.findViewById(R.id.left);
        forwardBtn = (Button)this.findViewById(R.id.forward);
        rightBtn = (Button)this.findViewById(R.id.right);

        leftBtn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                handler.obtainMessage(0x00).sendToTarget();
            }
        });

        forwardBtn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                handler.obtainMessage(0x01).sendToTarget();
            }
        });

        rightBtn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                handler.obtainMessage(0x02).sendToTarget();
            }
        });
    }

    private void connect() {
        new Thread() {
            @Override
            public void run() {
                EventLoopGroup workerLoopGroup = new NioEventLoopGroup();
                Bootstrap b = new Bootstrap();
                try {
                    //1 设置reactor 线程组
                    b.group(workerLoopGroup);
                    //2 设置nio类型的channel
                    b.channel(NioSocketChannel.class);
                    b.option(ChannelOption.SO_KEEPALIVE, true);
                    b.option(ChannelOption.TCP_NODELAY, true);
                    //3 设置监听端口
                    b.remoteAddress(address, port);
                    //4 设置通道的参数
                    b.option(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT);

                    //5 装配子通道流水线
                    b.handler(new ChannelInitializer<SocketChannel>() {
                        //有连接到达时会创建一个channel
                        protected void initChannel(SocketChannel ch) throws Exception {
                            // pipeline管理子通道channel中的Handler
                            // 向子channel流水线添加一个handler处理器
                            ch.pipeline().addLast(new MyLengthFieldBasedFrameDecoder(Utils.RECV_BUFF_LENGTH, 0, 4, 0, 4, handler));
                            ch.pipeline().addLast(new K210MsgDecoder());
                            ch.pipeline().addLast(new MsgHandler());
                        }
                    });
                    ChannelFuture f = b.connect();

                    f.addListener(new ChannelFutureListener(){
                        @Override
                        public void operationComplete(ChannelFuture futureListener) throws Exception {
                            if (futureListener.isSuccess()) {
                                Log.i("CONNECT OK", "EchoClient客户端连接成功!");

                            } else {
                                Log.i("CONNECT FAIL", "EchoClient客户端连接失败!");
                            }
                        }
                    });

                    // 阻塞,直到连接完成
                    f.sync();
                    channel = f.channel();
                    channel.closeFuture().sync();
                } catch (Exception e) {
                    Log.i("ERROR", "ERROR OCCURS !!!");
                    e.printStackTrace();
                } finally {
                    // 优雅关闭EventLoopGroup，
                    // 释放掉所有资源包括创建的线程
                    Log.i("ERROR", "RELEASE RESOURCE !");
//                    channel.close();
                    workerLoopGroup.shutdownGracefully();
                    handler.obtainMessage(0x03).sendToTarget();
                }
            }
        }.start();
    }

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case 0x00:
                    Log.i("Handler", "detect left");
                    byte[] cmd1 = {'C', 'M', 'D', 0x01, 0x21, 0x0f };
                    ByteBuf buffer1 = channel.alloc().buffer();
                    buffer1.writeBytes(cmd1);
                    channel.writeAndFlush(buffer1);
                    break;
                case 0x01:
                    Log.i("Handler", "detect forward");
                    byte[] cmd2 = {'C', 'M', 'D', 0x01, 0x20, 0x0f };
                    ByteBuf buffer2 = channel.alloc().buffer();
                    buffer2.writeBytes(cmd2);
                    channel.writeAndFlush(buffer2);
                    break;
                case 0x02:
                    Log.i("Handler", "detect right");
                    byte[] cmd3 = {'C', 'M', 'D', 0x01, 0x22, 0x0f };
                    ByteBuf buffer3 = channel.alloc().buffer();
                    buffer3.writeBytes(cmd3);
                    channel.writeAndFlush(buffer3);
                    break;
                case 0x03:
                    Log.i("Handler", "Reconnect ...");
                    connect();
                    break;
                default:
                    Log.i("Handler", "unknown command");
                    break;
            }
        }
    };
}
