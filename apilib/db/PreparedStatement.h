#ifndef PREPARED_STATEMENT_H
#define PREPARED_STATEMENT_H

#include "Define.h"
#include "SQLOperation.h"
#include <future>
#include <vector>

namespace DB
{
	class MySQLPreparedStatement;

	///////////////预设数据类型
	union PreparedStatementDataUnion
	{
		bool boolean;
		uint8 ui8;
		int8 i8;
		uint16 ui16;
		int16 i16;
		uint32 ui32;
		int32 i32;
		uint64 ui64;
		int64 i64;
		float f;
		double d;
	};

	enum PreparedStatementValueType
	{
		PSVT_BOOL,
		PSVT_UI8,
		PSVT_UI16,
		PSVT_UI32,
		PSVT_UI64,
		PSVT_I8,
		PSVT_I16,
		PSVT_I32,
		PSVT_I64,
		PSVT_FLOAT,
		PSVT_DOUBLE,
		PSVT_STRING,
		PSVT_BINARY,
		PSVT_NULL
	};

	struct PreparedStatementData
	{
		PreparedStatementDataUnion	data;
		PreparedStatementValueType	type;
		std::vector<uint8>			binary;
	};

	class LENDY_COMMON_API PreparedStatement
	{
		friend class PreparedStatementTask;
		friend class MySQLPreparedStatement;
		friend class MySQLConnection;

	public:
		PreparedStatement(uint32 index, uint8 capacity);

		virtual ~PreparedStatement();

		void SetNull(const uint8 index);

		void SetBool(const uint8 index, const bool value);

		void SetUInt8(const uint8 index, const uint8 value);

		void SetUInt16(const uint8 index, const uint16 value);

		void SetUInt32(const uint8 index, const uint32 value);

		void SetUInt64(const uint8 index, const uint64 value);

		void SetInt8(const uint8 index, const int8 value);

		void SetInt16(const uint8 index, const int16 value);

		void SetInt32(const uint8 index, const int32 value);

		void SetInt64(const uint8 index, const int64 value);

		void SetFloat(const uint8 index, const float value);

		void SetDouble(const uint8 index, const double value);

		void SetString(const uint8 index, const std::string& value);

		void SetBinary(const uint8 index, const std::vector<uint8>& value);

	protected:
		void BindParameters(MySQLPreparedStatement* stmt);

	protected:
		MySQLPreparedStatement * m_mpStmt;
		uint32 m_index; 
		EXPORT_BEGIN
		std::vector<PreparedStatementData> m_stdata;
		EXPORT_END
	
		DELETE_COPY_ASSIGN(PreparedStatement);
	};

	class MySQLPreparedStatement
	{
		friend class MySQLConnection;
		friend class PreparedStatement;

	public:
		MySQLPreparedStatement(MYSQL_STMT* stmt, std::string queryString);
		virtual ~MySQLPreparedStatement();

		void SetNull(const uint8 index);

		void SetBool(const uint8 index, const bool value);

		void SetUInt8(const uint8 index, const uint8 value);

		void SetUInt16(const uint8 index, const uint16 value);

		void SetUInt32(const uint8 index, const uint32 value);

		void SetUInt64(const uint8 index, const uint64 value);

		void SetInt8(const uint8 index, const int8 value);

		void SetInt16(const uint8 index, const int16 value);

		void SetInt32(const uint8 index, const int32 value);

		void SetInt64(const uint8 index, const int64 value);

		void SetFloat(const uint8 index, const float value);

		void SetDouble(const uint8 index, const double value);

		void SetBinary(const uint8 index, const std::vector<uint8>& value, bool isString);

		uint32 GetParameterCount() const { return m_paramCount; }

	protected:
		MYSQL_STMT* GetSTMT() { return m_mStmt; }
		MYSQL_BIND* GetBind() { return m_bind; }

		void ClearParameters();
		void AssertValidIndex(uint8 index);
		std::string GetQueryString() const;

	private:
		PreparedStatement*	m_pStmt;
		MYSQL_STMT*			m_mStmt;
		uint32				m_paramCount;
		std::vector<bool>	m_paramsSet;
		MYSQL_BIND*			m_bind;
		std::string const	m_queryString;

		DELETE_COPY_ASSIGN(MySQLPreparedStatement);
	};

	class PreparedStatementTask : public SQLOperation
	{
	public:
		PreparedStatementTask(PreparedStatement* stmt, bool async = false);
		virtual ~PreparedStatementTask();

		bool Execute() override;

		PreparedQueryResultFuture GetFuture() { return m_result->get_future(); }

	protected:
		PreparedStatement*			m_pStmt;
		bool						m_bResult;
		PreparedQueryResultPromise *m_result;
	};
}


#endif