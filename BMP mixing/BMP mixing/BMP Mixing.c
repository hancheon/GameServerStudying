#include <stdio.h>
#include <Windows.h>

int main()
{
    FILE* fp1 = fopen("sample.bmp", "rb");
    FILE* fp2 = fopen("sample2.bmp", "rb");
    FILE* fp3 = fopen("mixing.bmp", "wb");

    BITMAPFILEHEADER BitMapFileHeader;
    BITMAPINFOHEADER BitMapInfoHeader;

    // 비트맵 파일과 인포 헤더만큼 읽기
    int freadResult;

    freadResult = fread(&BitMapFileHeader, sizeof(BitMapFileHeader), 1, fp1);
    if (freadResult != 1)
    {
        printf("파일 읽기 실패\n");
    }

    freadResult = fread(&BitMapInfoHeader, sizeof(BitMapInfoHeader), 1, fp1);
    if (freadResult != 1)
    {
        printf("파일 읽기 실패\n");
    }

    freadResult = fread(&BitMapFileHeader, sizeof(BitMapFileHeader), 1, fp2);
    if (freadResult != 1)
    {
        printf("파일 읽기 실패\n");
    }

    freadResult = fread(&BitMapInfoHeader, sizeof(BitMapInfoHeader), 1, fp2);
    if (freadResult != 1)
    {
        printf("파일 읽기 실패\n");
    }

    // 비트맵 크기 계산
    int imageSize = BitMapInfoHeader.biHeight * BitMapInfoHeader.biWidth * (BitMapInfoHeader.biBitCount / 8);
    
    // 비트맵 크기만큼 동적 공간 할당
    char* pbuffer1 = malloc(imageSize);
    char* pbuffer2 = malloc(imageSize);
    char* pbufferB = malloc(imageSize);

    // 헤더 이후 비트맵 크기만큼 pImage에 비트맵 정보 저장
    if (fread(pbuffer1, imageSize, 1, fp1) != 1)
        printf("비트맵 읽기 실패\n");

    if (fread(pbuffer2, imageSize, 1, fp2) != 1)
        printf("비트맵 읽기 실패\n");

    // 동적 할당된 공간을 픽셀 단위(4바이트)로 접근하기 위한 포인터
    DWORD* pImage1 = (DWORD*)(pbuffer1);
    DWORD* pImage2 = (DWORD*)(pbuffer2);
    DWORD* pImageB = (DWORD*)(pbufferB);

    fwrite(&BitMapFileHeader, sizeof(BitMapFileHeader), 1, fp3);
    fwrite(&BitMapInfoHeader, sizeof(BitMapInfoHeader), 1, fp3);

    for (int i = 0; i < BitMapInfoHeader.biHeight; i++)
    {
        for (int j = 0; j < BitMapInfoHeader.biWidth; j++)
        {
            /*
            // 각 RGBA 분리 후 비트 단위 포인터 연산 방식
            // sample의 RGBA 분리 -> RGBA든 ARGB든 색 순서 상관없이 같은 색끼리 합성
            unsigned char R1 = *((char*)pImage1);
            unsigned char G1 = *((char*)pImage1 + 1);
            unsigned char B1 = *((char*)pImage1 + 2);
            unsigned char A1 = *((char*)pImage1 + 3);

            // sample2의 RGBA 분리
            unsigned char R2 = *((char*)pImage2);
            unsigned char G2 = *((char*)pImage2 + 1);
            unsigned char B2 = *((char*)pImage2 + 2);
            unsigned char A2 = *((char*)pImage2 + 3);

            // 합성 연산
            *((char*)pImageB) = (R1 / 2 + R2 / 2);
            *((char*)pImageB + 1) = (G1 / 2 + G2 / 2);
            *((char*)pImageB + 2) = (B1 / 2 + B2 / 2);
            *((char*)pImageB + 3) = (A1 / 2 + A2 / 2);

            // 각 RGBA 분리 후 비트 단위 비트 연산 방식 합성 연산
            *((char*)pImageB) = (R1 >> 1) + (R2 >> 1);
            *((char*)pImageB + 1) = (G1 >> 1) + (G2 >> 1);
            *((char*)pImageB + 2) = (B1 >> 1) + (B2 >> 1);
            *((char*)pImageB + 3) = (A1 >> 1) + (A2 >> 1); */

            // RGBA 분리없이 마스킹으로 픽셀 단위 합성
            // 0x7f7f7f7f를 &연산 하는 이유: 0x7f7f7f7f는 01111111 01111111 01111111 01111111
            // 픽셀 단위로 나누거나 비트 연산을 하면 RGBA끼리 침범 가능 -> 오른쪽 쉬프트 연산 후 첫 번째 비트는 반드시 0이니까 침범으로 1이되어도 &연산으로 0 고정
            *pImageB = ((*pImage1 >> 1) & 0x7f7f7f7f) + ((*pImage2 >> 1) & 0x7f7f7f7f);
            pImage1++;
            pImage2++;
            pImageB++;
        }
    }

    fwrite(pbufferB, imageSize, 1, fp3);

    free(pbuffer1);
    free(pbuffer2);
    free(pbufferB);

    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    return 0;
}