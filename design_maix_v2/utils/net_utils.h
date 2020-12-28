#ifndef __CLIENTUTILS_H__
#define __CLIENTUTILS_H__

#include "esp32_spi.h"
#include <stdio.h>
#include "unistd.h"
#include "htonx.h"

// 一次性发送数据块大小，建议长度为2048
#define SNDBUFSIZE 2048

// cmd msg: CMD 0x10 0x20 0xff
// 5 bytes
// 命令长度为5bytes，这里使用六个字节接收
#define MSGLEN 6
uint8_t msg[MSGLEN];

// // Maxmium number of socket
// #define	MAX_SOCK_NUM		4
// // Default state value for Wifi state field
// #define NA_STATE -1

// static int16_t _state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };
// static uint16_t _server_port[MAX_SOCK_NUM] = { 0, 0, 0, 0 };

// ping 服务器IP
uint8_t ip[] = {192, 168, 137, 1};

// void allocate_socket(uint8_t sock)
// {
//   _state[sock] = sock;
// }

// void release_socket(uint8_t sock)
// {
//   _state[sock] = NA_STATE;
// }

int available(uint8_t sock)
{
	if (sock != 255)
	{
		int bytes = esp32_spi_socket_available(sock);
		if (bytes>0)
		{
			return bytes;
		}
	}
	return 0;
}

int client_read_byte(uint8_t sock)
{
	uint8_t b;
	if (!available(sock))
		return -1;

	bool connClose = false;
	b = esp32_spi_get_data(sock);

	if (connClose)
	{
		// release_socket(sock);
		sock = 255;
	}

	return b;
}

int client_peek(uint8_t sock)
{
	uint8_t b;
	if (!available(sock))
		return -1;

	bool connClose = false;
	b = esp32_spi_get_data(sock);

	if (connClose)
	{
		// release_socket(sock);
		sock = 255;
	}

	return b;
}

void flush(uint8_t sock)
{
	while (available(sock))
		client_read_byte(sock);
}

int client_read(uint8_t sock, uint8_t* buf, size_t size)
{
	if (!available(sock))
		return -1;
	return esp32_spi_socket_read(sock, buf, size);
}

uint8_t client_status(uint8_t* sock)
{
	if (*sock == 255)
	{
		return SOCKET_CLOSED;
	}

	if (esp32_spi_socket_available(*sock) == 0)
	{
		return SOCKET_ESTABLISHED;
	}

	if (esp32_spi_socket_status(*sock) == SOCKET_ESTABLISHED)
	{
		return SOCKET_ESTABLISHED;
	}

    // release_socket(*sock);
	*sock = 255;

	return SOCKET_CLOSED;
}

uint8_t client_connected(uint8_t* sock)
{
	return (client_status(sock) == SOCKET_ESTABLISHED);
}

void client_stop(uint8_t sock)
{
	if (sock == 255)
		return;

    printk("Disconnecting %d\n", sock);
	esp32_spi_socket_close(sock);

	// release_socket(sock);
	sock = 255;
}

int send_to_client(uint8_t sock, uint8_t* img_buf_w_buf, uint32_t n_left)
{
    uint32_t n_written;
    while(n_left > 0)
    {
        if((n_written = esp32_spi_socket_write(sock, img_buf_w_buf, SNDBUFSIZE)) <= 0)
        {
            printf("wirte socket err1 !\n");
            return -1;
        }
        n_left -= n_written;
        img_buf_w_buf += n_written;
        if(n_left < SNDBUFSIZE)
        {
            if((n_written = esp32_spi_socket_write(sock, img_buf_w_buf, n_left)) <= 0)
            {
                printf("wirte socket err2 !\n");
                return -1;
            }
            break;
        }
    }
    return 0;
}

int test_network(void)
{
	printf("------------------------------------------\n");

    // int ping_www_count = 5;
    // int ping_www_success = 0;
    // while(ping_www_count--)
    // {
    //     ping_www_success = esp32_spi_ping("www.baidu.com", 1, 255);
    //     printf("ping www.baidu.com: %d ms\n", ping_www_success);
    //     if(ping_www_success > 0)
    //         break;
    //     sleep(1);
    // }

    // printf("------------------------------------------\n");

    int ping_ip_count = 5;
    int ping_ip_success = 0;
    while(ping_ip_count--)
    {
        ping_ip_success = esp32_spi_ping(ip, 0, 255);
        printf("ping 192.168.137.1: %d ms\n", ping_ip_success);
        if(ping_ip_success >= 0 && ping_ip_success < 255)
            break;
        sleep(1);
    }

    printf("------------------------------------------\n");
	return ping_ip_success;
}

int send_data(uint8_t sock, uint8_t* img_buf_w_buf, uint32_t n_left)
{
    uint32_t n_written;
    while(n_left > 0)
    {
        if((n_written = esp32_spi_socket_write(sock, img_buf_w_buf, SNDBUFSIZE)) <= 0)
        {
            printf("wirte socket err1 !\n");
            return -1;
        }
        n_left -= n_written;
        img_buf_w_buf += n_written;
		if(n_left < SNDBUFSIZE)
		{
			if((n_written = esp32_spi_socket_write(sock, img_buf_w_buf, n_left)) <= 0)
			{
				printf("wirte socket err2 !\n");
				return -1;
			}
			break;
		}
    }
    return 0;
}

#endif