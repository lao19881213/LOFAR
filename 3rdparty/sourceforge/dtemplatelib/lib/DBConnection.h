/* Copyright � 2000 
Michael Gradman and Corwin Joy 

Permission to use, copy, modify, distribute and sell this software and 
its documentation for any purpose is hereby granted without fee, provided 
that the above copyright notice appears in all copies and that both that 
copyright notice and this permission notice appear in supporting documentation. 
Corwin Joy and Michael Gradman make no representations about the suitability 
of this software for any purpose. 
It is provided "as is" without express or implied warranty.
*/ 
#ifndef DTL_DBCONNECTION_H
#define DTL_DBCONNECTION_H

#include "DB_Base.h"
#include "validate.h"

#ifdef  WIN32
#ifdef WIN32
	#ifndef DTL_USE_MFC
		#include <windows.h>
	#else 
		#include <afx.h>
	#endif
#endif

#endif
#include <sql.h>

// Revised: 11/12/2000 - CJ - cleaned up Connect() function
// Revised: 12/19/2000 - MG - added namespace support

BEGIN_DTL_NAMESPACE

class DBEnvironment {
protected:
	HENV henv;

	bool owns_environment;  // does this object own the handle?

public:
	DBEnvironment();
	~DBEnvironment();
	void Release();
	void Share(HENV new_henv);
	void init(); 
	HENV GetHENV() const;
	HENV GetHENV_NoThrow() const
	{
		return henv;
	}
};

class DBConnection : public ValidatedObject
{
public:
  // state of connection
  enum ConnState { CONN_UNALLOCATED, CONN_ALLOCATED, CONNECTED };
  // type of database
  enum DBType { DB_UNKNOWN, DB_ORACLE, DB_SQL_SERVER, DB_MYSQL, DB_OPENRDA,
				DB_ACCESS, DB_EXCEL
  };

protected:
  static DBEnvironment dbe;  // environment associated with this connection
  HDBC hdbc;  // this connection 
  bool auto_commit; // whether to autocommit or not
  ConnState state; // state of the connection
  bool owns_connection; // does this object own the handle?
  DBType dbType;
  tstring szQuoteChar;
  tstring szDriverName;
  tstring szDriverVer;
private:

  // prohibit copy construction and assignment
  DBConnection &operator=(const DBConnection &conn);

  DBConnection(const DBConnection &conn);
   
  // DSN we used to connect
  tstring usedDSN;

  static DBConnection defaultConn;

  // not responsible for allocating hdbc and dbe.henv,
  // that is done by the public version of Connect()
  void InternalConnect (const TCHAR *szConnStrIn);

  void ComputeDBType();

public:
  // connect to DB using the passed in DSN
  // and set the dbe.henv and hdbc members
  // throws exception if unable to connect!
  DBConnection(const tstring &DSN = _TEXT(""));

  // exception-safe swap()
  void swap(DBConnection &other);

  // Connect to the DB using the passed in DSN if not already connected
  // any existing constructs which reference this connection
  // become invalid if the DSN is different than the DBConnection was using
  // prior to the call to Connect()

  // connect using usedDSN as connection tstring if no arguments passed in
  void Connect();

  // cleanup issues here if exception is thrown!!!!
  void Connect(const tstring &DSN);

  // functions to commit or rollback all transactions that have occurred between last 
  // commit or rollback on this connection

  // turn autocommit on or off
  void SetAutoCommit(bool commit);

  bool GetAutoCommit();

  // only one mutating operation done, so should be exception-safe
  void CommitAll();

  // only one mutating operation done, so should be exception-safe
  // we require this to be no-throw for exception safe operations
  void RollbackAll();

  // share this HDBC with me
  void Share(HENV new_henv, HDBC new_hdbc);

	// cleanup issues here on exceptions!!!
  ~DBConnection();

  // only able to guarantee consistency
  // ODBC resources may leak
  void Release();

  static DBConnection &GetDefaultConnection();
  static DBEnvironment &GetDefaultEnvironment();

  // are we connected?
  bool IsConnected();

  friend class DBStmt;

  friend class DBException;

  HDBC GetHDBC() const;
  HENV GetHENV() const;
  tstring GetDSN() const;


  // kludge to get around header hell
  friend HDBC GetHDBC(const DBConnection &conn);
  friend HENV GetHENV(const DBConnection &conn);

  // return name of the DBMS being used
  tstring GetDBMSName() const;
  DBConnection::DBType GetDBMSEnum() const;


  // return the DBMS version being used
  tstring GetDBMSVersion() const;

  // return the character used by the driver to quote table names
  const tstring & GetQuoteChar(void) const;

  // return name of the odbc driver being used
  tstring GetDriverName() const;

  // return version of the odbc driver being used
  tstring GetDriverVersion() const;
};

HDBC GetHDBC(const DBConnection &conn);
HENV GetHENV(const DBConnection &conn);


END_DTL_NAMESPACE

#endif

