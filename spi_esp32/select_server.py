#!/usr/bin/env python3

from socket import *
import select

ADDR = '0.0.0.0'
PORT = 8096

def main():
	# 创建监听套接字
	server = socket(AF_INET, SOCK_STREAM)
	# 设置监听套接字为非阻塞
	server.setblocking(False)
	# 设置地址复用
	server.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
	# 绑定地址和端口
	server.bind((ADDR, PORT))
	# 监听
	server.listen(5)

	# read_fds 存放所有read事件发生的文件描述符，初始化将服务端socket对象加入监听列表
	read_fds=[server,]

	print('启动服务...')

	while True:
		# 启用select，如果监听套接字集合中有事件发生则返回，超时时间为1s，返回发生读、写和错误的文件描述符集合
		rfds, wfds, efds = select.select(read_fds, [], [], 1)
		# 遍历rfds，处理集合中套接字的“读事件”
		for sock in rfds:
			# socket 监听套接字上发生了读事件, 说明有客户端连接
			if sock == server:
				# accept 客户端的连接, 获取客户端套接字和地址
				conn, addr = sock.accept()
				print('client ', addr, ' connected ...')
				# 把新的客户端连接加入到“读事件”监听列表中，监听该客户端套接字上的“读事件”
				read_fds.append(conn)
			else:
				# 客户端套接字上有“读事件”发生，处理该客户端事件
				try:
					data = sock.recv(1024).decode('utf-8') # 读取客户端发来的数据
					peer = sock.getpeername()
					# read返回0，说明客户端关闭连接，将该客户端从监听列表中移除
					if not data:
						print('client ', peer, ' off line ...')
						sock.close()  # 关闭该套接字，释放资源
						read_fds.remove(sock)
						continue # 继续处理下一个套接字事件
					print('receive from ', peer, ' ,data is ', data)
					# 将消息回射给客户端
					sock.send(data.encode('utf-8'))
				#如果这个连接出错了，客户端暴力断开了（注意，我还没有接收他的消息，或者接收他的消息的过程中出错了）
				except Exception:
					print('Exception.......')
					# 关闭该连接
					sock.close()
					# 直接移除
					read_fds.remove(sock)
					
if __name__ == "__main__":
    main()