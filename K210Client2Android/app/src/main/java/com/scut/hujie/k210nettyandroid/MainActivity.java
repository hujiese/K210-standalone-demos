package com.scut.hujie.k210nettyandroid;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.scut.hujie.k210nettyandroid.decoder.K210MsgDecoder;
import com.scut.hujie.k210nettyandroid.decoder.MsgHandler;
import com.scut.hujie.k210nettyandroid.decoder.MyLengthFieldBasedFrameDecoder;
import com.scut.hujie.k210nettyandroid.utils.Const;

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


public class MainActivity extends AppCompatActivity {

    private Button leftBtn = null;
    private Button forwardBtn = null;
    private Button rightBtn = null;
    private Button servoBtn = null;
    private Button distanceBtn = null;
    private EditText editAngle = null;

    Channel channel;

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
        servoBtn = (Button)this.findViewById(R.id.servo);
        distanceBtn = (Button)this.findViewById(R.id.distance);
        editAngle = (EditText)this.findViewById(R.id.editAngle);

        leftBtn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                handler.obtainMessage(Const.DETECTLEFT).sendToTarget();
            }
        });

        forwardBtn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                handler.obtainMessage(Const.DETECTFORWARD).sendToTarget();
            }
        });

        rightBtn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                handler.obtainMessage(Const.DETECTRIGHT).sendToTarget();
            }
        });
        servoBtn.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                handler.obtainMessage(Const.SERVOMOVE).sendToTarget();
            }
        });
        distanceBtn.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                handler.obtainMessage(Const.DISTANCE).sendToTarget();
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
                    b.remoteAddress(Const.ADDRESS, Const.PORT);
                    //4 设置通道的参数
                    b.option(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT);

                    //5 装配子通道流水线
                    b.handler(new ChannelInitializer<SocketChannel>() {
                        //有连接到达时会创建一个channel
                        protected void initChannel(SocketChannel ch) throws Exception {
                            // pipeline管理子通道channel中的Handler
                            // 向子channel流水线添加一个handler处理器
                            ch.pipeline().addLast(new MyLengthFieldBasedFrameDecoder(Const.RECV_BUFF_LENGTH, 0, 4, 0, 4, handler));
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
                    workerLoopGroup.shutdownGracefully();
                    handler.obtainMessage(Const.RECONNECT).sendToTarget();
                }
            }
        }.start();
    }

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case Const.DETECTLEFT:
                    Log.i("Handler", "detect left");
                    sendCmdtoK210(Const.CMD_DETECT, Const.LEFT_DIRECTION);
                    break;
                case Const.DETECTFORWARD:
                    Log.i("Handler", "detect forward");
                    sendCmdtoK210(Const.CMD_DETECT, Const.FORWARD_DIRECTION);
                    break;
                case Const.DETECTRIGHT:
                    Log.i("Handler", "detect right");
                    sendCmdtoK210(Const.CMD_DETECT, Const.RIGHT_DIRECTION);
                    break;
                case Const.DISTANCE:
                    Log.i("Handler", "distance");
                    sendCmdtoK210(Const.CMD_DISTANCE, Const.EMP_ARG);
                    break;
                case Const.SERVOMOVE:
                    Log.i("Handler", "move servo");
                    String angleVlaue = editAngle.getText().toString().trim();
                    int angle = 180;
                    if(angleVlaue != null)
                        angle = Integer.valueOf(angleVlaue);
                    sendCmdtoK210(Const.CMD_SERVO, (byte)angle);
                    break;
                case Const.RECONNECT:
                    Log.i("Handler", "Reconnect ...");
                    connect();
                    break;
                default:
                    Log.i("Handler", "unknown command");
                    break;
            }
        }
    };
    public void sendCmdtoK210(byte cmd, byte arg){
        byte[] msg = {'C', 'M', 'D', cmd, arg, Const.CMD_END_FLAG };
        ByteBuf buffer = channel.alloc().buffer();
        buffer.writeBytes(msg);
        channel.writeAndFlush(buffer);
    }
}
