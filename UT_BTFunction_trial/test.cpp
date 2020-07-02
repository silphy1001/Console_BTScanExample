#include "pch.h"

#include <iostream>
#include <locale>
#include <WinSock2.h>
#include <ws2bth.h>

// Linker inputのライブラリパスがおかしかったのでプロジェクトのプロパティを修正
//   ---> gtest_maind.lib

// ビルド確認用コード
TEST(TestCaseName, TestName) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}

// Test Fixture Definition
class WinBTTrialForStartup : public ::testing::Test {
protected:
    WORD m_versionRequested;
    WSADATA m_wsaData;

    virtual void SetUp() {
        std::wcout.imbue(std::locale(""));
        m_versionRequested = MAKEWORD(2, 2);
        m_wsaData = { 0 };
    }

    virtual void TearDown() {
        WSACleanup();
    }
    // Helper methods
    void resultShow(int funcRetVal, const std::wstring& funcName)
    {
        if (funcRetVal) { // 戻り値が0でない時はエラー
            int wsaGLERet = WSAGetLastError();
            LPVOID msg;
            std::wcout << funcName << L" failed, returns: " << funcRetVal << std::endl;

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
                std::wcout << L"GetLastError() failed, returns: " << GetLastError() << std::endl;
            }
            else {
                std::wcout << (LPWSTR)msg << std::endl;
            }
            LocalFree(msg);
        }
        else {
            std::wcout << funcName << L" succeeded!" << std::endl;
        }
    }
};

class WinBTTrialForLookupServiceBegin : public WinBTTrialForStartup {
protected:
    DWORD m_dwControlFlags;
    BTH_QUERY_DEVICE m_queryDevParam;
    BLOB m_queryBlob;
    WSAQUERYSET m_queryParam;
    HANDLE m_hLookup;

    virtual void SetUp() {
        std::wcout.imbue(std::locale(""));

        // WSAStartup
        m_versionRequested = MAKEWORD(2, 2);
        m_wsaData = { 0 };
        WSAStartup(m_versionRequested, &m_wsaData);

        // WSALookupServiceBegin
        m_queryParam = { 0 };
        m_queryDevParam.LAP = 0; // Reserved. Must to be zero.
        m_queryDevParam.length = 10; // Requested length of the inquiry, in seconds.

        m_queryBlob.cbSize = sizeof(BTH_QUERY_DEVICE);
        m_queryBlob.pBlobData = (BYTE*)&m_queryDevParam;

        m_queryParam.dwSize = sizeof(WSAQUERYSET);
        m_queryParam.lpBlob = &m_queryBlob;
        m_queryParam.dwNameSpace = NS_BTH;

        // パラメータエラー10022L が出る 指定内容 要確認
        /*
        m_dwControlFlags = LUP_RETURN_ALL
            | LUP_NEAREST
            | LUP_FLUSHCACHE;
        */
        m_dwControlFlags = LUP_CONTAINERS | LUP_FLUSHCACHE | LUP_FLUSHPREVIOUS,

        m_hLookup = nullptr;
    }

    virtual void TearDown() {
        if (m_hLookup != nullptr) WSALookupServiceEnd(m_hLookup);
        WSACleanup();
    }
};

// Tests with test fixtures
TEST_F(WinBTTrialForStartup, WSAStartup_Success_Version22) {

    int ret = WSAStartup(m_versionRequested, &m_wsaData);
    //resultShow(ret, L"WSAStartup()");
    EXPECT_EQ(ret, 0);
}

TEST_F(WinBTTrialForStartup, WSAStartup_Success_Version21) {

    m_versionRequested = MAKEWORD(2, 1);
    m_wsaData = { 0 };

    int ret = WSAStartup(m_versionRequested, &m_wsaData);
    EXPECT_EQ(ret, 0);
}

TEST_F(WinBTTrialForStartup, WSAStartup_Success_Version20) {

    m_versionRequested = MAKEWORD(2, 0);
    m_wsaData = { 0 };

    int ret = WSAStartup(m_versionRequested, &m_wsaData);
    EXPECT_EQ(ret, 0);
}

TEST_F(WinBTTrialForStartup, WSAStartup_Success_Version11) {

    m_versionRequested = MAKEWORD(1, 1);
    m_wsaData = { 0 };

    int ret = WSAStartup(m_versionRequested, &m_wsaData);
    EXPECT_EQ(ret, 0);
}

TEST_F(WinBTTrialForStartup, WSAStartup_Success_Version10) {

    m_versionRequested = MAKEWORD(1, 0);
    m_wsaData = { 0 };

    int ret = WSAStartup(m_versionRequested, &m_wsaData);
    EXPECT_EQ(ret, 0);
}

TEST_F(WinBTTrialForStartup, DISABLED_WSAStartup_Fail_Version27) {

    m_versionRequested = MAKEWORD(2, 7);
    m_wsaData = { 0 };

    int retWSAStartup = WSAStartup(m_versionRequested, &m_wsaData);
    int retWSAGetLastError = WSAGetLastError();
    EXPECT_NE(retWSAStartup, 0);
    EXPECT_EQ(retWSAGetLastError, WSAVERNOTSUPPORTED);
}

TEST_F(WinBTTrialForStartup, DISABLED_WSAStartup_Fail_NullptrWSAData) {

    int retWSAStartup = WSAStartup(m_versionRequested, nullptr);
    int retWSAGetLastError = WSAGetLastError();
    EXPECT_NE(retWSAStartup, 0);
    EXPECT_EQ(retWSAGetLastError, WSAEFAULT);
}

TEST_F(WinBTTrialForLookupServiceBegin, WSALookupServiceBegin_Success) {

    int retWSALookupServiceBegin = WSALookupServiceBegin(
        &m_queryParam,
        m_dwControlFlags,
        &m_hLookup);
    int retWSAGetLastError = WSAGetLastError();
    EXPECT_EQ(retWSALookupServiceBegin, 0);
    EXPECT_EQ(retWSAGetLastError, 0);
}

TEST_F(WinBTTrialForLookupServiceBegin, WSALookupServiceBegin_Fail_NotEnoughMemory) {

    WSAQUERYSET tmp;
    int retWSALookupServiceBegin = WSALookupServiceBegin(
        &tmp,
        m_dwControlFlags,
        &m_hLookup);
    int retWSAGetLastError = WSAGetLastError();
    EXPECT_EQ(retWSALookupServiceBegin, SOCKET_ERROR);
    EXPECT_EQ(retWSAGetLastError, WSAEFAULT);
}

TEST_F(WinBTTrialForLookupServiceBegin, WSALookupServiceBegin_Fail_NoWSAStartup) {

    EXPECT_EQ(WSACleanup(), 0);

    int retWSALookupServiceBegin = WSALookupServiceBegin(
        &m_queryParam,
        m_dwControlFlags,
        &m_hLookup);
    int retWSAGetLastError = WSAGetLastError();
    EXPECT_EQ(retWSALookupServiceBegin, SOCKET_ERROR);
    EXPECT_EQ(retWSAGetLastError, WSANOTINITIALISED);
}
// Test without test fixtures
