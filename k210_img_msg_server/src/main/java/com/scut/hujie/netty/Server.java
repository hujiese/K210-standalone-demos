package com.scut.hujie.netty;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.List;
import java.util.logging.Logger;

/**
 * create by jackster
 **/
public class Server {

    private final int serverPort;
    private ServerBootstrap b = new ServerBootstrap();
    public Logger logger = Logger.getLogger(this.getClass().getName());

    public Server(int port) {
        this.serverPort = port;
    }

    public void runServer() {
        //创建reactor 线程组
        EventLoopGroup bossLoopGroup = new NioEventLoopGroup(1);
        EventLoopGroup workerLoopGroup = new NioEventLoopGroup();

        try {
            //1 设置reactor 线程组
            b.group(bossLoopGroup, workerLoopGroup);
            //2 设置nio类型的channel
            b.channel(NioServerSocketChannel.class);
            //3 设置监听端口
            b.localAddress(serverPort);
            //4 设置通道的参数
            b.option(ChannelOption.SO_KEEPALIVE, true);
            b.option(ChannelOption.TCP_NODELAY, true);
            b.option(ChannelOption.SO_REUSEADDR, true);
            b.option(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT);
            b.childOption(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT);

            //5 装配子通道流水线
            b.childHandler(new ChannelInitializer<SocketChannel>() {
                //有连接到达时会创建一个channel
                protected void initChannel(SocketChannel ch) throws Exception {
                    // pipeline管理子通道channel中的Handler
                    // 向子channel流水线添加3个handler处理器
                    ch.pipeline().addLast(new LengthFieldBasedFrameDecoder(Utils.RECV_BUFF_LENGTH, 0, 4, 0, 4));
                    ch.pipeline().addLast(new K210MsgDecoder());
                    ch.pipeline().addLast(new MsgHandler());
                }
            });
            // 6 开始绑定server
            // 通过调用sync同步方法阻塞直到绑定成功
            ChannelFuture channelFuture = b.bind().sync();
            logger.info(" 服务器启动成功，监听端口: " +
                    channelFuture.channel().localAddress());

            // 7 等待通道关闭的异步任务结束
            // 服务监听通道会一直等待通道关闭的异步任务结束
            ChannelFuture closeFuture = channelFuture.channel().closeFuture();
            closeFuture.sync();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            // 8 优雅关闭EventLoopGroup，
            // 释放掉所有资源包括创建的线程
            workerLoopGroup.shutdownGracefully();
            bossLoopGroup.shutdownGracefully();
        }

    }

    class K210MsgDecoder extends ByteToMessageDecoder {
        @Override
        public void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
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
                K210Msg msg = new K210Msg(1);
                msg.setMsgOk(false);
                out.add(msg);
            }
        }
    }

    //服务器端业务处理器
    static class MsgHandler extends ChannelInboundHandlerAdapter {
        public Logger logger = Logger.getLogger(this.getClass().getName());
        @Override
        public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
            K210Msg k210Msg = (K210Msg)msg;
            if(k210Msg.isMsgOk()) {
                logger.info(msg.toString());
                nioCopyFile(Utils.IMG_FULL_PATH, k210Msg.getJpegImage().length, k210Msg.getJpegImage());
            }
        }
       public void nioCopyFile(String path, int length, byte[] img) {
           try {
               File file = new File(path);
               FileOutputStream out = null;
               FileChannel fileChannel = null;
               try {
                   if(!file.exists()){
                       file.createNewFile();
                   }
                   out = new FileOutputStream(file);
                   fileChannel = out.getChannel();
                   ByteBuffer byteBuffer = ByteBuffer.allocate(length);
                   byteBuffer.put(img);
                   byteBuffer.flip();
                   fileChannel.write(byteBuffer);
                   byteBuffer.clear();
                   fileChannel.force(true);
               } finally {
                   fileChannel.close();
                   out.close();
               }
           } catch (Exception e){
               e.printStackTrace();
           }
       }

    }


    public static void main(String[] args) throws InterruptedException {
        int port = Utils.SOCKET_SERVER_PORT;
        new Server(port).runServer();
    }


}
