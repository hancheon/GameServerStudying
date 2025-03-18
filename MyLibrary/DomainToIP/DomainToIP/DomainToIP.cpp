#include <iostream>
#include <WS2tcpip.h>

BOOL DomainToIP(wchar_t* domain, IN_ADDR* addr)
{
    ADDRINFOW* addrInfo;
    SOCKADDR_IN* sockAddr;

    if (GetAddrInfo(domain, L"0", NULL, &addrInfo) != 0)
    {
        return FALSE;
    }

    sockAddr = (SOCKADDR_IN*)addrInfo->ai_addr;
    *addr = sockAddr->sin_addr;
    FreeAddrInfo(addrInfo);
    return TRUE;
}