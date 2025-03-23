#include <iostream>

char* IDLfileName;
int fileNameSize;
char* messageBuffer;
int fileSize;

void makeHeaderFile();
void makeSourceFile();

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

    makeHeaderFile();
    makeSourceFile();
}

void makeHeaderFile()
{
    FILE* headerFile;
    char* fileName = new char[fileNameSize + 3];
    strcpy_s(fileName, fileNameSize + 3, IDLfileName);
    strcat_s(fileName, fileNameSize + 3, ".h");
    fopen_s(&headerFile, fileName, "wb");
    if (headerFile == nullptr)
        return;

    const char* includes = "#include <Windows.h>\r\n#include \"Packet.h\"\r\n\r\n";
    fwrite(includes, strlen(includes), 1, headerFile);
 
    int pType;
    char* func;
    char* context = nullptr;
    while (1)
    {
        pType = atoi(strtok_s(messageBuffer, " : ", &context));
        func = strtok_s(nullptr, "\r\n", &context);
    }

    fwrite(messageBuffer, fileSize, 1, headerFile);

    delete[] fileName;
}

void makeSourceFile()
{
    FILE* sourceFile;
    char* fileName = new char[fileNameSize + 5];
    strcpy_s(fileName, fileNameSize + 5, IDLfileName);
    strcat_s(fileName, fileNameSize + 5, ".cpp");
    fopen_s(&sourceFile, fileName, "wb");
    if (sourceFile == nullptr)
        return;

    fwrite(messageBuffer, fileSize, 1, sourceFile);

    delete[] fileName;
}
