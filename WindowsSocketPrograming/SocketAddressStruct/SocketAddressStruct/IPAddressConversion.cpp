#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#include <format>
#include <string>
#include <array>

void ipAddressConversionExample()
{
	// IPv4 ���ڿ� �ּ� -> ���̳ʸ�
	in_addr binaryIPv4{};
	const char* ipv4String = "192.168.0.1";

	if (inet_pton(AF_INET, ipv4String, &binaryIPv4) != 1)
	{
		std::cerr << "IPv4 �ּ� ��ȯ ����\n";
		return;
	}

	std::cout << std::format("IPv4 ���̳ʸ�: 0x{:X}\n", binaryIPv4.S_un.S_addr);

	// IPv4 ���̳ʸ� -> ���ڿ�
	std::array<char, INET_ADDRSTRLEN> ipv4StringBuf{};
	if (inet_ntop(AF_INET, &binaryIPv4, ipv4StringBuf.data(), ipv4StringBuf.size()) == nullptr)
	{
		std::cerr << "IPv4 �ּ� ���ڿ� ��ȯ ����\n";
		return;
	}

	std::cout << std::format("IPv4 ���ڿ�: {}\n", ipv4StringBuf.data());

	// IPv6 ���ڿ� -> ���̳ʸ�
	in6_addr binaryIPv6{};
	const char* ipv6String = "2001:db8::1428:57ab";

	if (inet_pton(AF_INET6, ipv6String, &binaryIPv6) != 1)
	{
		std::cerr << "IPv6 �ּ� ��ȯ ����\n";
		return;
	}

	// IPv6 ���̳ʸ� -> ���ڿ�
	std::array<char, INET6_ADDRSTRLEN> ipv6StringBuf{};
	if (inet_ntop(AF_INET6, &binaryIPv6, ipv6StringBuf.data(), ipv6StringBuf.size()) == nullptr)
	{
		std::cerr << "IPv6 �ּ� ���ڿ� ��ȯ ����\n";
		return;
	}

	std::cout << std::format("IPv6 ���ڿ�: {}\n", ipv6StringBuf.data());
}

int main()
{
	ipAddressConversionExample();
}