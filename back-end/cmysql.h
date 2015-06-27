/************************************************************
  FileName: 		cmysql.h
  Description:
  				该文件中申明了读写数据库的函数
  History:
      				sagezou    2008-02-28     1.0     新建
  Other:
  				CDN开发组公用的读写数据库的函数
***************************/

#if !defined _CMYSQL_H
#define _CMYSQL_H

#include <string>
#include <map>
#include <vector>
#include <mysql.h>

#include "cexception.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////
//	Caution:
//		1. 使用CMysql类必须捕获CMysqlException异常
////////////////////////////////////////////////////////////////////////////////////

class CMysqlException: public CCommonException
{
public:
    CMysqlException(const char* sErrMsg):CCommonException(sErrMsg) {};
    CMysqlException(const char* sErrMsg, const char* sUrl):CCommonException(sErrMsg, sUrl) {};
};

typedef map<string, int> STRING2INT;

class CMysql
{
public:
    CMysql();
    CMysql(const char* szHost, const char* szUser, const char* szPass, const char* szDB = NULL, unsigned short usPort = 0);
    ~CMysql();

    int Close();
    int Connect(const char* szHost, const char* szUser, const char* szPass, const char* szDB = NULL, unsigned short usPort = 0);
    int Connect();
    bool IfConnected(const char* szHost);

    int BeginWork();
    int Commit();
    int RollBack();

    int Query(const char* szSqlString);
    int StoreResult();
    int FreeResult();
    char** FetchRow();
    const char* GetFieldName(int iField);
    char* GetField(unsigned int iField);
    char* GetField(int iField);
    char* GetField(const char* szFieldName);
    unsigned int GetRowsNum();
    unsigned int GetAffectedRows();
    unsigned int GetAffectedCols()
    {
        return m_iField;
    };
    MYSQL* GetConnectHandle()
    {
        return &m_connection;
    }
    const char*  GetLastError()
    {
        return m_ErrMsg;
    }
    unsigned int GetLastErrNo()
    {
        return m_nLastErrNo;
    }

    static int EscapeString (string &str);

    static int EscapeString (const char* dat, const int len, string &str);

    unsigned int GetLastInsertId();
    unsigned int  m_iRows;

    unsigned long GetFieldLength (const char* szFieldName);

    unsigned long GetFieldLength (int rowID);


private:
    char m_ErrMsg[1024];
    char m_szHost[64];	// 数据库主机名
    char m_szUser[64];	// 数据库用户名
    char m_szPass[64];	// 数据库用户密码
    char m_szDB[64]; // 默认选中的数据库
    unsigned short m_usDBPort; // 数据库端口

    unsigned int  m_iField;
    MYSQL m_connection;
    MYSQL_RES *m_result;
    MYSQL_ROW m_row;
    STRING2INT m_FieldIndex;
    bool m_bFieldIndexInitialized;
    bool m_bConnected;
    unsigned int   m_nLastErrNo;

    unsigned long  *m_FieldLengths;

    void InitFieldLength();

    int InitFieldName();
};

#endif //_CMYSQL_H
