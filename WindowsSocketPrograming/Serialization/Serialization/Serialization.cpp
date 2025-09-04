// Serialization.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <fstream>
#include <cstring>

// 구조체 패킹을 사용한 직렬화
#pragma pack(push, 1) // 기존 정렬 제거 후 1바이트 정렬로 변경 -> 패딩 제거
struct PackedData {
    int id;
    float value;
    char name[16];
    bool active;
    double score;
};
#pragma pack(pop)

// 복합 데이터 구조체
struct GamePlayer {
    int playerId;
    char playerName[32];
    float health;
    float mana;
    int level;
    bool isOnline;
};
#pragma pack(pop)

int main()
{
    std::cout << "=== 구조체 패킹 직렬화 예제 ===" << std::endl;

    // 단일 구조체 직렬화
    PackedData data1 = { 123, 45.67f, "TestData", true, 98.5 };

    std::cout << "원본 데이터:" << std::endl;
    std::cout << "ID: " << data1.id << std::endl;
    std::cout << "Value: " << data1.value << std::endl;
    std::cout << "Name: " << data1.name << std::endl;
    std::cout << "Active: " << data1.active << std::endl;
    std::cout << "Score: " << data1.score << std::endl;

    // 메모리 직렬화 (바이트 배열로)
    std::cout << "\n=== 메모리 직렬화 ===" << std::endl;

    char buffer[sizeof(PackedData)];
    std::memcpy(buffer, &data1, sizeof(PackedData));

    PackedData data3;
    std::memcpy(&data3, buffer, sizeof(PackedData));

    std::cout << "메모리에서 복사된 데이터: " << data3.name << std::endl;

    // 구조체 크기 정보
    std::cout << "\n=== 구조체 크기 정보 ===" << std::endl;
    std::cout << "PackedData 크기: " << sizeof(PackedData) << " bytes" << std::endl;

    return 0;
}

// 결과
// sizeof(PackedData): 33 byte
// why? int (4) float (4) char [16] (16) bool (1) double (8) => 33