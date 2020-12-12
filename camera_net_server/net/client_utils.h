#ifndef __CLIENTUTILS_H__
#define __CLIENTUTILS_H__

#include "esp32_spi.h"
#include <stdio.h>
#include "unistd.h"

// Maxmium number of socket
#define	MAX_SOCK_NUM		4
// Default state value for Wifi state field
#define NA_STATE -1

static int16_t _state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };
static uint16_t _server_port[MAX_SOCK_NUM] = { 0, 0, 0, 0 };

void allocate_socket(uint8_t sock)
{
  _state[sock] = sock;
}

void release_socket(uint8_t sock)
{
  _state[sock] = NA_STATE;
}

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
		release_socket(sock);
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
		release_socket(sock);
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

    release_socket(*sock);
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

	release_socket(sock);
	sock = 255;
}
#endif