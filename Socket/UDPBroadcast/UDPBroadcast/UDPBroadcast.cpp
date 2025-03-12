#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#include <Mstcpip.h>
#pragma comment(lib, "ws2_32")

#define PROTOCOL_PACKETSIZE 10
#define BUF_SIZE 100

//프로토콜 - 0xff 0xee 0xdd 0xaa 0x00 0x99 0x77 0x55 0x33 0x11   (10 Byte)

int wmain()
{
	int retval;

	setlocale(LC_ALL, "Korean");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("%d\n", GetLastError());
		return -1;
	}

	// 브로드캐스팅 활성화
	BOOL isBroadcast = TRUE;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&isBroadcast, sizeof(isBroadcast));
	if (retval == SOCKET_ERROR)
	{
		printf("%d\n", GetLastError());
		return -1;
	}

	u_long non = 1;
	ioctlsocket(sock, FIONBIO, &non);

	char packet[PROTOCOL_PACKETSIZE];
	packet[0] = 0xff;
	packet[1] = 0xee;
	packet[2] = 0xdd;
	packet[3] = 0xaa;
	packet[4] = 0x00;
	packet[5] = 0x99;
	packet[6] = 0x77;
	packet[7] = 0x55;
	packet[8] = 0x33;
	packet[9] = 0x11;

	// 브로드캐스팅 주소 설정
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	// 서버의 주소 받을 구조체
	SOCKADDR_IN roomAddr;
	int addrLen = sizeof(roomAddr);
	wchar_t roomInfo[BUF_SIZE];
	wchar_t ip[16];

	timeval time;
	time.tv_sec = 0;
	time.tv_usec = 200;

	for (int i = 0; i <= 100; i++)
	{
		addr.sin_port = htons(10000 + i);

		retval = sendto(sock, packet, PROTOCOL_PACKETSIZE, 0, (SOCKADDR*)&addr, sizeof(addr));
		if (retval == SOCKET_ERROR)
		{
			printf("%d\n", GetLastError());
			return -1;
		}
		printf("%d 바이트 전송\n", retval);

		FD_SET rSet;
		FD_ZERO(&rSet);
		FD_SET(sock, &rSet);
		select(0, &rSet, nullptr, nullptr, &time);

		retval = recvfrom(sock, (char*)roomInfo, BUF_SIZE, 0, (SOCKADDR*)&roomAddr, &addrLen);
		if (retval == SOCKET_ERROR)
		{
			printf("%d\n", GetLastError());
			return -1;
		}

		roomInfo[retval / 2] = L'\0';

		InetNtop(AF_INET, &roomAddr.sin_addr, ip, 16);
		wprintf(L"[%s:%u] %s\n", ip, ntohs(roomAddr.sin_port), roomInfo);
	}

}