package com.scut.hujie.netty;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.LengthFieldPrepender;
import io.netty.handler.codec.string.StringEncoder;
import io.netty.util.CharsetUtil;

import java.io.File;
import java.io.FileInputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.logging.Logger;

/**
 * create by jackster
 **/
public class Client {
    private int serverPort;
    private String serverIp;
    private Bootstrap b = new Bootstrap();
    public Logger logger = Logger.getLogger(this.getClass().getName());

    public Client(String ip, int port) {
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

            //5 装配通道流水线
            b.handler(new ChannelInitializer<SocketChannel>() {
                //初始化客户端channel
                protected void initChannel(SocketChannel ch) throws Exception {
                    // 客户端channel流水线添加2个handler处理器
                    ch.pipeline().addLast(new LengthFieldPrepender(4));
                    ch.pipeline().addLast(new StringEncoder(CharsetUtil.UTF_8));
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

            try {
                File srcFile = new File(Utils.READ_IMG_FULL_PATH);
                int imgLen = (int)srcFile.length();
                FileInputStream fis = null;
                FileChannel inChannel = null;
                try {
                    fis = new FileInputStream(srcFile);
                    inChannel = fis.getChannel();

                    ByteBuffer buf = ByteBuffer.allocate(imgLen);
                    inChannel.read(buf);
                    buf.flip();
                    ByteBuf buffer = channel.alloc().buffer();
                    byte[] partMsg = {0x66, 0x02, 0x01, 0x01, 0x00};
                    buffer.writeInt(partMsg.length + 4 + imgLen);
                    buffer.writeBytes(partMsg);
                    buffer.writeInt(154);
                    buffer.writeBytes(buf);
                    channel.writeAndFlush(buffer);
                    buf.clear();
                } catch (Exception e){
                    e.printStackTrace();
                } finally {
                    inChannel.close();
                    fis.close();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

            channel.flush();

            // 7 等待通道关闭的异步任务结束
            // 服务监听通道会一直等待通道关闭的异步任务结束
            ChannelFuture closeFuture = channel.closeFuture();
            closeFuture.sync();

        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            // 优雅关闭EventLoopGroup，
            // 释放掉所有资源包括创建的线程
            workerLoopGroup.shutdownGracefully();
        }

    }

    public static void main(String[] args) throws InterruptedException {
        int port = Utils.SOCKET_SERVER_PORT;
        String ip = Utils.SOCKET_SERVER_IP;
        new Client(ip, port).runClient();
    }
}