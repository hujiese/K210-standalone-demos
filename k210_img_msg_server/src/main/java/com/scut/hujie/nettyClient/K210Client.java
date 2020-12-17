package com.scut.hujie.nettyClient;

import com.scut.hujie.netty.Utils;
import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;

import java.util.Scanner;
import java.util.logging.Logger;

public class K210Client {

    public boolean errFlag = false;
    private Logger logger = Logger.getLogger(this.getClass().getName());
    private int serverPort;
    private String serverIp;
    Bootstrap b = new Bootstrap();

    public K210Client(String ip, int port) {
        this.serverPort = port;
        this.serverIp = ip;
    }

    public void runClient() {
        //创建reactor 线程组
        EventLoopGroup workerLoopGroup = new NioEventLoopGroup();

        try {
            //1 设置reactor 线程组
            b.group(workerLoopGroup);
            //2 设置nio类型的channel
            b.channel(NioSocketChannel.class);
            //3 设置监听端口
            b.remoteAddress(serverIp, serverPort);
            //4 设置通道的参数
            b.option(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT);

            //5 装配子通道流水线
            b.handler(new ChannelInitializer<SocketChannel>() {
                //有连接到达时会创建一个channel
                protected void initChannel(SocketChannel ch) throws Exception {
                    // pipeline管理子通道channel中的Handler
                    // 向子channel流水线添加一个handler处理器
                    ch.pipeline().addLast(new LengthFieldBasedFrameDecoder(Utils.RECV_BUFF_LENGTH, 0, 4, 0, 4));
                    ch.pipeline().addLast(new K210MsgDecoder());
                    ch.pipeline().addLast(new MsgHandler());
                }
            });
            ChannelFuture f = b.connect();
            f.addListener((ChannelFuture futureListener) ->
            {
                if (futureListener.isSuccess()) {
                    logger.info("EchoClient客户端连接成功!");

                } else {
                    logger.info("EchoClient客户端连接失败!");
                }

            });

            // 阻塞,直到连接完成
            f.sync();
            Channel channel = f.channel();

            Scanner scanner = new Scanner(System.in);
            while (scanner.hasNext()) {
                System.out.print("Please input your command: ");
                String cmd = scanner.nextLine().trim();
                // cmd msg: CMD 0x10 0x20 0x0f
                byte[] cmds = {'C', 'M', 'D', 0x10, 0x28, 0x0f };
                if(cmd.equals("L")){
                    cmds[4] = 0x21;
                }else if(cmd.equals("F")){
                    cmds[4] = 0x20;
                }else if(cmd.equals("R")){
                    cmds[4] = 0x22;
                }
                //发送ByteBuf
                ByteBuf buffer = channel.alloc().buffer();
                buffer.writeBytes(cmds);
                channel.writeAndFlush(buffer);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            // 优雅关闭EventLoopGroup，
            // 释放掉所有资源包括创建的线程
            workerLoopGroup.shutdownGracefully();
        }

    }

    public static void main(String[] args) throws InterruptedException {
        int port = 8080;
        String ip = "192.168.137.238";
        new K210Client(ip, port).runClient();
    }
}