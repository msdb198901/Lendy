/*
 * Copyright (C) 2008-2019 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FIELD_H
#define _FIELD_H

#include "Define.h"
#include "DBEnvHeader.h"
#include <vector>

enum class DatabaseFieldTypes : uint8
{
    Null,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Decimal,
    Date,
    Binary
};

namespace DB
{
	class LENDY_COMMON_API Field
	{
		friend class ResultSet;
		friend class PreparedResultSet;

	public:
		Field();
		~Field();

		bool GetBool() const // Wrapper, actually gets integer
		{
			return GetUInt8() == 1 ? true : false;
		}

		uint8 GetUInt8() const;
		int8 GetInt8() const;
		uint16 GetUInt16() const;
		int16 GetInt16() const;
		uint32 GetUInt32() const;
		int32 GetInt32() const;
		uint64 GetUInt64() const;
		int64 GetInt64() const;
		float GetFloat() const;
		double GetDouble() const;
		char const* GetCString() const;
		std::string GetString() const;
		std::vector<uint8> GetBinary() const;

		bool IsNull() const
		{
			return data.value == nullptr;
		}

		struct Metadata
		{
			char const* TableName;
			char const* TableAlias;
			char const* Name;
			char const* Alias;
			char const* Type;
			uint32 Index;
		};

	protected:
#pragma pack(push, 1)
		struct
		{
			uint32 length;            // Length (prepared strings only)
			void* value;              // Actual data in memory
			DatabaseFieldTypes type;  // Field type
			bool raw;                 // Raw bytes? (Prepared statement or ad hoc)
		} data;
#pragma pack(pop)

		void SetByteValue(void* newValue, DatabaseFieldTypes newType, uint32 length);
		void SetStructuredValue(char* newValue, DatabaseFieldTypes newType, uint32 length);

		void CleanUp()
		{
			// Field does not own the data if fetched with prepared statement
			if (!data.raw)
				delete[]((char*)data.value);
			data.value = nullptr;
		}

		bool IsType(DatabaseFieldTypes type) const;

		bool IsNumeric() const;

	private:
#ifdef TRINITY_STRICT_DATABASE_TYPE_CHECKS
		void SetMetadata(MYSQL_FIELD* field, uint32 fieldIndex);
		Metadata meta;
#endif
	};

}

#endif
