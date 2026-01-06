#include <iostream>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

template <typename T>
bool SetSockOpt(SOCKET sock, int level, int optname, const T& optval)
{
    return setsockopt(sock, level, optname, reinterpret_cast<const char*>(&optval), sizeof(T)) != SOCKET_ERROR;
}

template <typename T>
bool GetSockOpt(SOCKET s, int level, int optname, T& optval)
{
    int optlen = sizeof(T);
    return getsockopt(sock, level, optname, reinterpret_cast<char*>(&optval), &optlen) != SOCKET_ERROR;
}

int main()
{
    SOCKET sock;

    //------SO_KEEPALIVE------//
    BOOL keepAlive = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&keepAlive), sizeof(keepAlive));
    SetSockOpt(sock, SOL_SOCKET, SO_KEEPALIVE, keepAlive);

    //------SO_LINGER------//
    linger lin;
    lin.l_onoff = 1;
    lin.l_linger = 0;
    setsockopt(sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>(&lin), sizeof(lin));
    SetSockOpt(sock, SOL_SOCKET, SO_LINGER, lin);

    //------SO_SNDBUF & SO_RCVBUF------//
    int sendBufSize = 64 * 1024; // 64kb
    int recvBufSize = 64 * 1024; // 64kb
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(sendBufSize), sizeof(sendBufSize));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(recvBufSize), sizeof(recvBufSize));
    SetSockOpt(sock, SOL_SOCKET, SO_SNDBUF, sendBufSize);
    SetSockOpt(sock, SOL_SOCKET, SO_RCVBUF, recvBufSize);
}