#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "myList.h"
#include "Protocol.h"
#pragma comment(lib, "ws2_32.lib")

// 전역 변수
SOCKET listen_sock;

// Player 구조체
struct PLAYER
{
    SOCKET socket;
    wchar_t ipAddr[16];
    int port;
    int starID;
    int xpos;
    int ypos;
    bool isAlive;
};

bool RecvProc();
bool SendUnicast(PLAYER, char*); // 보낼 대상 버퍼
bool SendBroadcast(PLAYER, char*); // 제외할 대상 버퍼
bool Disconnect();


int main()
{
    while (1)
    {

        네트워크()

            로직()

            랜더링() → 좌표 동기화 확인용(전용 클라로 만드는 경우도 있음) 해도 되고 안해도 되고
            → 나 : 윈도우로 창 만들어보자 ?

    }
}

ID = Null;

리슨 소켓 생성 바인딩 리슨

while (1)
{

    네트워크()

    로직()

    랜더링() → 좌표 동기화 확인용(전용 클라로 만드는 경우도 있음) 해도 되고 안해도 되고
    → 나 : 윈도우로 창 만들어보자 ?

}

bool networking()
{
    FD_SET readSet;
    FD_ZERO(readSet);
    FD_SET(리슨, &readSet);

    for (PlayerList ~) // 리슨 소켓에 들어온 클라 연결 요청을 유저로 승격
    {
        FD_SET(Player→Socket, &readSet);
    }
    // 타임아웃: 요청한 기능을 못할 때 최대 대기시간
    // timeval = NULL로 설정하는 이유: 어차피 서버 로직 없으니까 항상 기다리고 있어도 괜찮음
    ret = select(0, readSet, nullptr, nullptr, NULL);
   
    if (FD_ISSET(리슨, &readSet))
    {
        acceptProc();
    }

    for (PlayerList - ) // ret값이랑 recv 된 수랑 비교해서 반복 횟수 줄이기 가능
    {
        if (FP_ISSET(PLAYER->socket, &readSet))
        {
            recvProc(Player);
        }
    }
}

bool acceptProc()
{
    sock = accept();
    // Player 생성 세팅 ID 부여
    // ID 할당 메세지 send
    sendUnicast(Player, &allocID);
    // Player 리스트에 추가
    PlayerList.push();
    // 접속자에 대한 기존 접속자들 별 생성 메세지 전체 send
    STAR_CREATE.type = 1;
    // 기존 유저에게 새로운 접속자에게 별 생성 메세지 send


    // 서버는 unicast, broadcast(UDP 브로드캐스팅x, 유저 전체한테 보내기), 전체 중 일부 빼고 send 유형

}

bool sendUnicast(Player, buffer) // 보낼 대상 버퍼
{

}

bool sendBroadcast(Player, buffer) // 제외할 대상 버퍼
{

}


bool RecvProc() // -> 플레이어 리스트 순환 중
{
    char buffer[16] = { 0 };
    ret = recv();
    if (연결처리)
    {
        disconnect
    }
    
    int type = *(int*)buffer;
    switch (static_cast<MSG_TYPE>(type))
    {
        case MSG_TYPE::ID_ALLOC:
            break;
        case MSG_TYPE::STAR_CREATE:
            break;
        case MSG_TYPE::STAR_DELETE:
            break;
        case MSG_TYPE::STAR_MOVE:
            break;
        default:
            break;
    }
}

bool Disconnect()
{
    // 플레이어 리스트에서 해당 플레이어 제거 -> 문제: RecvProc에서 리스트 이터레이터 순환 중 -> 댕글링 포인터 발생
    // 여기서 제거를 안해야 하는 거임 -> 지연 삭제
    해당 플레이어.isAlive = false; // 후 나중에 한번에 삭제
    // 해당 플레이어 별 삭제 메세지 SendBroadcast -> 이 함수 안에서도 Disconnect가 일어날 수 있음 -> 재귀 발생

    // 
}

// 릴리즈말고 디버그로 왜? 댕글링 포인터 에러가 안나서 디버그는 할당 해제한 메모리를 0xdd 다 밀어주니까