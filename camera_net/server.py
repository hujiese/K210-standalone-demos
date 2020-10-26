#!/usr/bin/python3

# 导入 socket、sys 模块
import socket
import sys
import struct

# 端口
port = 8096
# 缓冲区大小
BUFSIZ = 1024
# 本机地址
host = '0.0.0.0'

def main():
	# 创建 socket 对象
	serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 

	# 设置地址复用
	serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	# 设置保活时间
	serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
	serversocket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 2)
	serversocket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 1)
	serversocket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3)
		
	# 绑定端口号
	serversocket.bind((host, port))
	# 设置最大连接数，超过后排队
	serversocket.listen(5)

	count = 0

	while True:
		# 建立客户端连接
		conn, addr = serversocket.accept()  
		print('connect from:', addr)
		while True: 
			count = count + 1
			imgName = str(count) + '.jpeg'
			fp = open(imgName, 'wb')
			try:
				data = conn.recv(4) # 读取客户端发来的数据
				peer = conn.getpeername()
				# read返回0，说明客户端关闭连接，将该客户端从监听列表中移除
				if not data:
					print('client ', peer, ' off line ...')
					conn.close()
					break
				dataLen = struct.unpack('!I', data)[0]
				print('receive from ', peer, ' ,data length is : ', dataLen)

				img = b''
				part = b''
				while dataLen:
					part = conn.recv(dataLen)
					# img += part
					dataLen = dataLen - len(part)
					print("len size = ", len(part))
					fp.write(part)
				fp.close()
				# print(img)
				print('write img ok ...')
			except Exception:
				print('Time out ...')	
	serversocket.close()
	
if __name__ == "__main__":
    main()