#include <iostream>
#include <string>

char* IDLfileName;
int fileNameSize;
char* messageBuffer;
int fileSize;

void makeProxyHeader();
void makeProxySource();

int main(int argc, char* argv[])
{
	FILE* message;
	fopen_s(&message, argv[1], "rb");
	if (message == nullptr)
		return -1;

	fseek(message, 0, SEEK_END);
	fileSize = ftell(message);
	fseek(message, 0, SEEK_SET);

	messageBuffer = new char[fileSize];
	fread(messageBuffer, fileSize, 1, message);

	char* context = nullptr;
	IDLfileName = strtok_s(argv[1], ".", &context);
	fileNameSize = strlen(IDLfileName);

	makeProxyHeader();
	makeProxySource();
}

void makeProxyHeader()
{
	FILE* headerFile;
	const char* fileType = "Proxy.h";
	int len = fileNameSize + strlen(fileType) + 1;
	char* fileName = new char[len];
	strcpy_s(fileName, len, IDLfileName);
	strcat_s(fileName, len, fileType);
	fopen_s(&headerFile, fileName, "wb");
	if (headerFile == nullptr)
		return;

	const char* includes = "#pragma once\r\n#include <Windows.h>\r\n#include \"Packet.h\"\r\n\r\n";
	fwrite(includes, strlen(includes), 1, headerFile);

	char* context = nullptr;
	strtok_s(messageBuffer, "}", &context);
	while (1)
	{
		int pType;
		const char* funcType = "void ";
		char* func;

		pType = atoi(strtok_s(messageBuffer, "  ", &context));
		strtok_s(nullptr, " ", &context);
		func = strtok_s(nullptr, "\r\n", &context);
		len = strlen(func) + strlen(funcType) + 1;
		char* funcDeclaration = new char[len];
		strcpy_s(funcDeclaration, len, funcType);
		strcat_s(funcDeclaration, len, func);
		fwrite(funcDeclaration, strlen(funcDeclaration), 1, headerFile);

		delete[] funcDeclaration;
	}

	delete[] fileName;
}

void makeProxySource()
{
	FILE* sourceFile;
	const char* fileType = "Proxy.cpp";
	int len = fileNameSize + strlen(fileType) + 1;
	char* fileName = new char[len];
	strcpy_s(fileName, len, IDLfileName);
	strcat_s(fileName, len, fileType);
	fopen_s(&sourceFile, fileName, "wb");
	if (sourceFile == nullptr)
		return;

	fwrite(messageBuffer, fileSize, 1, sourceFile);

	delete[] fileName;
}
