/************************************************************
  FileName: 		cmysql.cpp
  Description:     
  				该文件定义了读写数据库的函数
  History:        	
      				sagezou    2008-02-28     1.0     新建
  Other:
  				CDN开发组公用的读写数据库的函数
***************************/
#include <stdio.h>
#include <string.h>
#include "cmysql.h"

using namespace std;

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
CMysql:: CMysql()
{
	m_bConnected = false;
	m_iField = 0;
	m_result = NULL;
	bzero (m_szHost, sizeof(m_szHost));
	bzero (m_szUser, sizeof(m_szUser));
	bzero (m_szPass, sizeof(m_szPass));
	bzero (m_szDB, sizeof(m_szDB));
	m_usDBPort = 0;
	m_bFieldIndexInitialized = false;
	m_nLastErrNo = 0;
}

CMysql:: CMysql(const char* szHost, const char* szUser, const char* szPass, const char* szDB, unsigned short usPort)
{
	m_bConnected = false;
	m_iField = 0;
	m_result = NULL;
	strncpy (m_szHost, szHost, sizeof(m_szHost));
	strncpy (m_szUser, szUser, sizeof(m_szUser));
	strncpy (m_szPass, szPass, sizeof(m_szPass));
	if (szDB)
	{
		strncpy(m_szDB, szDB, sizeof(m_szDB));
		m_szDB[sizeof(m_szDB) - 1] = 0;
	}
	else
	{
		bzero (m_szDB, sizeof(m_szDB));
	}
	m_usDBPort = usPort;
	m_bFieldIndexInitialized = false;
}

CMysql:: ~CMysql()
{
	Close();
}

int CMysql:: Close()
{
	if (m_bConnected)
	{
		FreeResult();
		mysql_close(&m_connection);
		m_bConnected= false;
	}
	return 0;
}

int CMysql:: EscapeString (string &str)
{
	if (str.size()==0) return 0;

	char *buff= new char[str.size()*2+1];
	mysql_escape_string (buff, str.c_str(), str.size());
	str= buff;
	delete[] buff;

	return 0;
}

int CMysql:: EscapeString(const char* dat, const int len, string &str)
{
	if (dat == NULL) return 0;

	char *buff = new char[len*2+1];
	mysql_escape_string (buff, dat, len);
	str = buff;
	delete[] buff;

	return 0;
}

int CMysql:: Connect(const char* szHost, const char* szUser, const char* szPass, const char* szDB, unsigned short usPort)
{
	if (!strcmp(szHost, m_szHost))
		return Connect();
	strncpy (m_szHost, szHost, sizeof(m_szHost));
	strncpy (m_szUser, szUser, sizeof(m_szUser));
	strncpy (m_szPass, szPass, sizeof(m_szPass));
	if (szDB)
	{
		strncpy(m_szDB, szDB, sizeof(m_szDB));
		m_szDB[sizeof(m_szDB) - 1] = 0;
	}
	else
	{
		bzero (m_szDB, sizeof(m_szDB));
	}
	m_usDBPort = usPort;
	Close();
	return Connect();
}

int CMysql:: Connect()
{
	if (!m_bConnected)
	{
		mysql_init (&m_connection);
        my_bool b = 1;
        mysql_options(&m_connection, MYSQL_OPT_RECONNECT, &b);
		if (mysql_real_connect(&m_connection, m_szHost, m_szUser, m_szPass, m_szDB[0] ? m_szDB : NULL, m_usDBPort, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
		{
			m_nLastErrNo = mysql_errno(&m_connection);
			snprintf(m_ErrMsg, sizeof(m_ErrMsg)-1, "connect[-h%s -u%s -p%s] fail.\nError %u (%s)\n",
					m_szHost, m_szUser, m_szPass,
					mysql_errno(&m_connection), mysql_error(&m_connection));
			throw CMysqlException(m_ErrMsg);
		}
		m_bConnected = true;
	}
	return 0;
}

bool CMysql:: IfConnected(const char* szHost)
{
	if (m_bConnected)
		if (!strcmp(szHost, m_szHost))
			return true;
	return false;
}

int CMysql:: Query(const char* szSqlString)
{
	Connect();

	if (mysql_real_query(&m_connection, szSqlString, strlen(szSqlString)) != 0)
	{
		m_nLastErrNo = mysql_errno(&m_connection);
		snprintf(m_ErrMsg, sizeof(m_ErrMsg)-1, "query fail [%s].\nError=[%s]\nSQL=%s",
			m_szHost, mysql_error(&m_connection), szSqlString);
		m_ErrMsg[sizeof(m_ErrMsg) - 1] = 0;
		
		throw CMysqlException(m_ErrMsg);
	}
	return 0;
}

int CMysql:: FreeResult()
{
	if (m_result != NULL)
		mysql_free_result (m_result);
	m_iField = 0;
	m_result = NULL;
	if (m_bFieldIndexInitialized)
	{
		m_FieldIndex.clear();
		m_bFieldIndexInitialized = false;
	}
	return 0;
}

int CMysql:: StoreResult()
{
	FreeResult();
	m_result = mysql_store_result (&m_connection);
	if (m_result == NULL)
	{
		m_nLastErrNo = mysql_errno(&m_connection);
		snprintf(m_ErrMsg, sizeof(m_ErrMsg)-1, "store_result fail:%s!", mysql_error(&m_connection));
		m_ErrMsg[sizeof(m_ErrMsg) - 1] = 0;
		
		throw CMysqlException(m_ErrMsg);
	}
	m_iField = mysql_num_fields (m_result);
	m_iRows = mysql_num_rows (m_result);
	mysql_next_result(&m_connection);
	return 0;
}

char** CMysql:: FetchRow()
{
	if (m_result == NULL)
		StoreResult();
	m_row = mysql_fetch_row (m_result);

	m_FieldLengths= NULL;
	return m_row;
}

int CMysql:: InitFieldName()
{
	if ((!m_bFieldIndexInitialized) && (m_result!=NULL))
	{
		unsigned int i;
		MYSQL_FIELD *fields;

		fields = mysql_fetch_fields(m_result);
		for(i = 0; i < m_iField; i++)
		{
			m_FieldIndex[fields[i].name] = i;
		}
		m_bFieldIndexInitialized = true;
	}
	return 0;
}

const char* CMysql:: GetFieldName(int iField)
{
	if (m_result==NULL) {
		return NULL;
	}
	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(m_result);
	if ((unsigned int)iField> m_iField) {
		return NULL;
	}
	return fields[iField].name;
}

/*
	返回受影响的行数
*/
unsigned int CMysql:: GetAffectedRows()
{
	my_ulonglong iNumRows;

	if (!m_bConnected) return 0;
	iNumRows = mysql_affected_rows(&m_connection);

	return (unsigned int)iNumRows;
}
/*
	返回查询结果集中的行数
*/
unsigned int CMysql:: GetRowsNum()
{
	my_ulonglong iNumRows;

	if (!m_bConnected || m_result == NULL)
		return 0;
	iNumRows = mysql_num_rows(m_result);

	return (unsigned int)iNumRows;
}

/* 返回最近一次insert的ID
*/
unsigned int CMysql:: GetLastInsertId ()
{
	return (unsigned int)mysql_insert_id(&m_connection);
}

unsigned long CMysql:: GetFieldLength (const char* szFieldName)
{
	InitFieldName();

	return GetFieldLength(m_FieldIndex[szFieldName]);
}

unsigned long CMysql:: GetFieldLength (int rowID)
{
	InitFieldLength();

	return m_FieldLengths[rowID];
}

void CMysql:: InitFieldLength()
{
	if (m_FieldLengths != NULL) return;

	m_FieldLengths= mysql_fetch_lengths(m_result);
}

/*
	按照字段名取回当前行的结果
*/
char* CMysql:: GetField(const char* szFieldName)
{
	InitFieldName();
	return GetField(m_FieldIndex[szFieldName]);
}

/*
	按照字段索引取回当前行的结果
*/
char* CMysql:: GetField(unsigned int iField)
{
	if (iField > m_iField)
		return NULL;
	return m_row[iField];
}

/*
	按照字段索引取回当前行的结果
*/
char* CMysql:: GetField(int iField)
{
	if ((unsigned int)iField > m_iField)
		return NULL;
	return m_row[iField];
}

int CMysql::BeginWork()
{
	return Query("BEGIN WORK");
}

int CMysql::Commit()
{
	return Query("COMMIT");
}

int CMysql::RollBack()
{
	return Query("ROLLBACK");
}


