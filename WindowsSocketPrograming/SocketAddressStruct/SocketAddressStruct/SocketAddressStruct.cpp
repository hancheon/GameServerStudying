// SocketAddressStruct.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <span>

void initIPv4Address()
{
    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "192.168.0.1", &serverAddr.sin_addr);
}

void initIPv6Address()
{
    sockaddr_in6 serverAddr{};

    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(12345);
    serverAddr.sin6_flowinfo = 0;
    inet_pton(AF_INET, "192.168.0.1", &serverAddr.sin6_addr);
    serverAddr.sin6_scope_id = 0;
}

void protocolIndependentExample()
{
    sockaddr_storage remoteAddr{};

    if (remoteAddr.ss_family == AF_INET) { // ipv4 주소 처리
        auto* ipv4Addr = reinterpret_cast<sockaddr_in*>(&remoteAddr);
    }
    else if (remoteAddr.ss_family == AF_INET6) { // ipv6 주소 처리
        auto* ipv6Addr = reinterpret_cast<sockaddr_in6*>(&remoteAddr);
    }
}

int main()
{
    // 기본 소켓 주소 구조 (sockaddr)
    // 형태
    // typedef struct sockaddr {
    //      ADDRESS_FAMILY sa_family;
    //      CHAR sa_data[14]; 
    // } SOCKADDR, *PSOCKADDR, FAR *LPSOCKADDR;
    sockaddr basic{};

    // IPv4 소켓 주소 구조체 (sockaddr_in)
    // 형태
    // typedef struct sockaddr_in {
    //      ADDRESS_FAMILY sin_family;
    //      USHORT sin_port;
    //      IN_ADDR sin_addr;
    //      CHAR sin_zero[8];
    //  } SOCKADDR_IN, * PSOCKADDR_IN;
    sockaddr_in ipv4{};

    // IPv6 소켓 주소 구조체 (sockaddr_in6)
    // 형태
    // typedef struct sockaddr_in {
    //      ADDRESS_FAMILY sin6_family;
    //      USHORT sin6_port;
    //      ULONG  sin6_flowinfo;
    //      IN6_ADDR sin6_addr; 
    //      union {
    //          ULONG sin6_scope_id;     // Set of interfaces for a scope.
    //          SCOPE_ID sin6_scope_struct;
    //      }
    //  } SOCKADDR_IN6_LH, *PSOCKADDR_IN6_LH, FAR *LPSOCKADDR_IN6_LH;
    sockaddr_in6 ipv6{};

    // 프로토콜 독립적인 소켓 주소 구조체 (sockaddr_storage)
    // 형태
    // typedef struct sockaddr_storage {
    //      ADDRESS_FAMILY ss_family;
    //      CHAR __ss_pad1[_SS_PAD1SIZE];
    //      __int64 __ss_align;
    //      CHAR __ss_pad2[_SS_PAD2SIZE];
    // } SOCKADDR_STORAGE_LH, *PSOCKADDR_STORAGE_LH, FAR *LPSOCKADDR_STORAGE_LH;
    sockaddr_storage both{};
}