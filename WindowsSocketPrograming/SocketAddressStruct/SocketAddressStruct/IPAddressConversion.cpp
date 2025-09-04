#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#include <format>
#include <string>
#include <array>

void ipAddressConversionExample()
{
	// IPv4 문자열 주소 -> 바이너리
	in_addr binaryIPv4{};
	const char* ipv4String = "192.168.0.1";

	if (inet_pton(AF_INET, ipv4String, &binaryIPv4) != 1)
	{
		std::cerr << "IPv4 주소 변환 실패\n";
		return;
	}

	std::cout << std::format("IPv4 바이너리: 0x{:X}\n", binaryIPv4.S_un.S_addr);

	// IPv4 바이너리 -> 문자열
	std::array<char, INET_ADDRSTRLEN> ipv4StringBuf{};
	if (inet_ntop(AF_INET, &binaryIPv4, ipv4StringBuf.data(), ipv4StringBuf.size()) == nullptr)
	{
		std::cerr << "IPv4 주소 문자열 변환 실패\n";
		return;
	}

	std::cout << std::format("IPv4 문자열: {}\n", ipv4StringBuf.data());

	// IPv6 문자열 -> 바이너리
	in6_addr binaryIPv6{};
	const char* ipv6String = "2001:db8::1428:57ab";

	if (inet_pton(AF_INET6, ipv6String, &binaryIPv6) != 1)
	{
		std::cerr << "IPv6 주소 변환 실패\n";
		return;
	}

	// IPv6 바이너리 -> 문자열
	std::array<char, INET6_ADDRSTRLEN> ipv6StringBuf{};
	if (inet_ntop(AF_INET6, &binaryIPv6, ipv6StringBuf.data(), ipv6StringBuf.size()) == nullptr)
	{
		std::cerr << "IPv6 주소 문자열 변환 실패\n";
		return;
	}

	std::cout << std::format("IPv6 문자열: {}\n", ipv6StringBuf.data());
}

int main()
{
	ipAddressConversionExample();
}