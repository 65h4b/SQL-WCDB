//
// Created by sanhuazhang on 2019/05/19
//

/*
 * Tencent is pleased to support the open source community by making
 * WCDB available.
 *
 * Copyright (C) 2017 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <WCDB/ColumnType.hpp>
#include <WCDB/HandleRelated.hpp>
#include <WCDB/SQLiteDeclaration.h>
#include <WCDB/Statement.hpp>
#include <WCDB/Value.hpp>

namespace WCDB {

class InnerHandleStatement : public HandleRelated {
    friend class AbstractHandle;

public:
    InnerHandleStatement() = delete;
    InnerHandleStatement(const InnerHandleStatement &) = delete;
    InnerHandleStatement &operator=(const InnerHandleStatement &) = delete;

    InnerHandleStatement(InnerHandleStatement &&other);
    InnerHandleStatement(AbstractHandle *handle);

    void enableTableMonitor();

    virtual ~InnerHandleStatement() override;

    virtual bool prepare(const Statement &statement);
    virtual bool prepare(const UnsafeStringView &sql);
    virtual bool isPrepared();
    virtual void finalize();

    virtual bool step();
    virtual bool done();
    virtual void reset();

    using Integer = ColumnTypeInfo<ColumnType::Integer>::UnderlyingType;
    using Text = ColumnTypeInfo<ColumnType::Text>::UnderlyingType;
    using Float = ColumnTypeInfo<ColumnType::Float>::UnderlyingType;
    using BLOB = ColumnTypeInfo<ColumnType::BLOB>::UnderlyingType;

    virtual void bindInteger(const Integer &value, int index = 1);
    virtual void bindDouble(const Float &value, int index = 1);
    virtual void bindText(const Text &value, int index = 1);
    virtual void bindBLOB(const BLOB &value, int index = 1);
    virtual void bindNull(int index = 1);
    virtual void
    bindPointer(void *ptr, int index, const Text &type, void (*destructor)(void *));
    int bindParameterIndex(const Text &parameterName);

    virtual void bindValue(const Value &value, int index = 1);
    virtual void bindRow(const OneRowValue &row);

    virtual ColumnType getType(int index = 0);
    virtual signed long long getColumnSize(int index = 0);
    virtual int getNumberOfColumns();

    virtual Integer getInteger(int index = 0);
    virtual Float getDouble(int index = 0);
    virtual Text getText(int index = 0);
    virtual const BLOB getBLOB(int index = 0);

    virtual Value getValue(int index = 0);
    virtual OneColumnValue getOneColumn(int index = 0);
    virtual OneRowValue getOneRow();
    virtual MultiRowsValue getAllRows();

    virtual const UnsafeStringView getOriginColumnName(int index);
    virtual const UnsafeStringView getColumnName(int index);
    virtual const UnsafeStringView getColumnTableName(int index);

    virtual bool isReadOnly();

protected:
    virtual bool isBusy();

private:
    void analysisStatement(const Statement &statement);
    sqlite3_stmt *m_stmt;
    bool m_done;
    StringView m_newTable;
    StringView m_modifiedTable;
};

} //namespace WCDB