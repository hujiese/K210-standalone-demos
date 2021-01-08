#ifndef __HOSTTONETX_H__
#define __HOSTTONETX_H__
 
typedef unsigned short uint16;
typedef unsigned long  uint32;
//短整形高低字节交换
#define Swap16(A) ((((uint16)(A) & 0xff00) >> 8) | (((uint16)(A) & 0x00ff) << 8))
//长整形高低字节交换
#define Swap32(A) ((((uint32)(A) & 0xff000000) >> 24) | \
				   (((uint32)(A) & 0x00ff0000) >>  8) | \
				   (((uint32)(A) & 0x0000ff00) <<  8) | \
				   (((uint32)(A) & 0x000000ff) << 24))
 
static union {   
    char c[4];   
    unsigned long mylong;   
} endian_test = {{ 'l', '?', '?', 'b' } };  
 
/******************************************************************************
ENDIANNESS返回结果
	l:小端模式
	b:打断模式
******************************************************************************/
#define ENDIANNESS ((char)endian_test.mylong)  
 
//将主机的无符号短整形数转换成网络字节顺序
uint16 htons(uint16 hs)
{
	return (ENDIANNESS=='l') ? Swap16(hs): hs;
}
 
//将主机的无符号长整形数转换成网络字节顺序
uint32 htonl(uint32 hl)
{
	return (ENDIANNESS=='l') ? Swap32(hl): hl;
}
 
//将一个无符号短整形数从网络字节顺序转换为主机字节顺序
uint16 ntohs(uint16 ns)
{
	return (ENDIANNESS=='l') ? Swap16(ns): ns;	
}
 
//将一个无符号长整形数从网络字节顺序转换为主机字节顺序
uint32 ntohl(uint32 nl)
{
	return (ENDIANNESS=='l') ? Swap32(nl): nl;	
}
#endif