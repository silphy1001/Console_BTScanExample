// Console_BTScanExample.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <memory>
#include <WinSock2.h>
#include <ws2bth.h>
#include <WS2tcpip.h>


int main()
{
    std::cout << "Hello Bluetooth World!\n";

    // アクセス方法
    //   Windows SocketsにBT SDPがマッピングされる
    WSADATA wsaData = { 0 };
    LPVOID lpMsgBuf = 0;
    int ret = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (ret != 0) {
        int wsaErr = WSAGetLastError();
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            wsaErr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,
            nullptr);
        std::cout << (LPCTSTR)lpMsgBuf << std::endl;
        LocalFree(lpMsgBuf);
        return EXIT_FAILURE;
    }

    // 大まかにやりたいこと
    //   1.近くのデバイスの検出
    BTH_QUERY_DEVICE queryDevParam;
    queryDevParam.LAP = 0; // Reserved. Must to be zero.
    queryDevParam.length = 10; // Requested length of the inquiry, in seconds.

    BLOB queryBlob;
    queryBlob.cbSize = sizeof(BTH_QUERY_DEVICE); // Size
    queryBlob.pBlobData = (BYTE *)&queryDevParam; // BlobData

    WSAQUERYSET queryParam = { 0 };
    // 下記のパラメータ以外はWSALookupServiceBegin()関数では無視される
    queryParam.dwSize = sizeof(WSAQUERYSET); // Size
    queryParam.lpBlob = &queryBlob; // Blob
    queryParam.dwNameSpace = NS_BTH; // NameSpace

    HANDLE hLookup = { 0 };

    ret = WSALookupServiceBegin(
        &queryParam,
        LUP_CONTAINERS | LUP_FLUSHCACHE | LUP_FLUSHPREVIOUS,
        &hLookup);
    if (ret != 0) {
        int wsaErr = WSAGetLastError();
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            wsaErr,
            MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),
            (LPTSTR)&lpMsgBuf,
            0,
            nullptr);
        std::cout << (LPCTSTR)lpMsgBuf << std::endl;
        LocalFree(lpMsgBuf);
        WSACleanup();
        return EXIT_FAILURE;
    }

    //   2.検出したデバイスのリスト化 -> オブジェクト化しておいて後段のプログラムから扱えるようにしたい
    while (true) {

        WSAQUERYSET wsaQuery = { 0 };
        WCHAR wsServiceInstanceName[512] = { 0 };
        wsaQuery.lpszServiceInstanceName = wsServiceInstanceName;
        GUID guid = { 0 };
        wsaQuery.lpServiceClassId = &guid;
        WSAVERSION version = { 0 };
        wsaQuery.lpVersion = &version;
        WCHAR wsComment[512] = { 0 };
        wsaQuery.lpszComment = wsComment;
        GUID providerId = { 0 };
        wsaQuery.lpNSProviderId = &providerId;
        AFPROTOCOLS afProtocols = { 0 };
        wsaQuery.lpafpProtocols = &afProtocols;
        WCHAR szQueryString[512] = { 0 };
        wsaQuery.lpszQueryString = szQueryString;
        CSADDR_INFO csaddr_info = { 0 };
        wsaQuery.lpcsaBuffer = &csaddr_info;
        BLOB blob = { 0 };
        wsaQuery.lpBlob = &blob;

        DWORD dwBufferLength = sizeof(wsaQuery);

        ret = WSALookupServiceNext(
        hLookup,
        LUP_NEAREST
        | LUP_RETURN_NAME
        | LUP_RETURN_TYPE
        | LUP_RETURN_COMMENT
        | LUP_RETURN_ADDR
        | LUP_FLUSHCACHE
        | LUP_FLUSHPREVIOUS,
        &dwBufferLength,
        &wsaQuery);
        
        int wsaErr = WSAGetLastError();
        if (ret != 0 && wsaErr == WSA_E_NO_MORE) {
            std::cout << "No more available data, finish process." << std::endl;
            break;
        }
        else if (ret != 0 && wsaErr == WSANO_DATA) {
            std::cout << "No data, finish process." << std::endl;
            break;
        }
        else if (ret != 0 && wsaErr == WSA_E_CANCELLED) {
            std::cout << "Connection is canceled, abort program." << std::endl;
            if (WSALookupServiceEnd(hLookup) != 0) {
                int wsaErr = WSAGetLastError();
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM
                    | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    wsaErr,
                    MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),
                    (LPTSTR)&lpMsgBuf,
                    0,
                    nullptr);
                std::cout << (LPCTSTR)lpMsgBuf << std::endl;
                LocalFree(lpMsgBuf);
            }
            WSACleanup();
            return EXIT_FAILURE;
        }
        else if (ret != 0) { // 他のエラーは捕捉してメッセージを出して次の周回へ
            int wsaErr = WSAGetLastError();
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                wsaErr,
                MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),
                (LPTSTR)&lpMsgBuf,
                0,
                nullptr);
            std::cout << (LPCTSTR)lpMsgBuf << std::endl;
            LocalFree(lpMsgBuf);
            continue;
        }

        // 成功したら読み出し
        std::cout << "wsaQuery.lpcsaBuffer: " << wsaQuery.lpcsaBuffer << std::endl;
        std::cout << "wsaQuery.lpszServiceInstanceName: " << wsaQuery.lpszServiceInstanceName << std::endl;
        std::cout << "wsaQuery.lpServiceClassId: " << wsaQuery.lpServiceClassId << std::endl;
        std::cout << "wsaQuery.lpszComment: " << wsaQuery.lpszComment << std::endl;
    }
    //   3.デバイスへの接続
    //   4.デバイスでサポートしているサービスやデバイスの特性の列挙
    //   5.特性の読み取りと書き込み
    //   6.特性値が変化した時の通知の受信登録

    // 終了処理
    if (WSALookupServiceEnd(hLookup) != 0) {
        int wsaErr = WSAGetLastError();
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            wsaErr,
            MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),
            (LPTSTR)&lpMsgBuf,
            0,
            nullptr);
        std::cout << (LPCTSTR)lpMsgBuf << std::endl;
        LocalFree(lpMsgBuf);
    }
    WSACleanup();

    return EXIT_SUCCESS;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//   1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
