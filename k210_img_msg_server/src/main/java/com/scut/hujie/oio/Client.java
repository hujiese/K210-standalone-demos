package com.scut.hujie.oio;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Scanner;

// 用于测试k210作为服务端，该客户端建立长连接后通过读取终端命令给k210发送CMD
public class Client {
    public static void main(String[] args) {
        Socket s = null;
        try {
            s = new Socket("192.168.137.238", 8080);
            Scanner scaner = new Scanner(System.in);
            while(true){
                OutputStream os = s.getOutputStream();
                System.out.print("Please input your command: ");
                String cmd = scaner.nextLine().trim();
                // cmd msg: CMD 0x10 0x20 0x0f
                byte[] cmds = {'C', 'M', 'D', 0x10, 0x28, 0x0f };
                if(cmd.equals("L")){
                    cmds[4] = 0x21;
                }else if(cmd.equals("F")){
                    cmds[4] = 0x20;
                }else if(cmd.equals("R")){
                    cmds[4] = 0x22;
                }
                os.write(cmds);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            try {
                s.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
