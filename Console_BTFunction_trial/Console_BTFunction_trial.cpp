// Console_BTFunction_trial.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <WinSock2.h>
#include <ws2bth.h>

void resultShow(int wsaRetVal, const std::string &funcName)
{
    if (wsaRetVal) { // 戻り値が0でない時はエラー
        int wsaGLERet = WSAGetLastError();
        LPVOID msg;
        std::cout << funcName << " failed." << std::endl;

        int fmRet = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            wsaGLERet,
            MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),
            (LPWSTR)&msg,
            256,
            nullptr
        );
        if (!fmRet) {
            std::cout << "GetLastError() failed, returns: " << GetLastError() << std::endl;
        }
        else {
            std::cout << msg << std::endl;
        }
    }
    else {
        std::cout << funcName << " succeeded!" << std::endl;
    }
}

int main(void)
{
    std::cout << "Hello Bluetooth World!" << std::endl;

    WORD versionRequested = MAKEWORD(2, 2);
    WSADATA wsaData = { 0 };
    int ret = 0;

    ret = WSAStartup(versionRequested, &wsaData);
    resultShow(ret, "WSAStartup()");

    ret = WSACleanup();
    resultShow(ret, "WSACleanup()");

    return EXIT_SUCCESS;
}
