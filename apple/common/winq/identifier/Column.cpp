//
// Created by sanhuazhang on 2019/05/02
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

#include <WCDB/WINQ.h>

namespace WCDB {

Column::Column() = default;

Column::Column(const UnsafeStringView& name)
{
    syntax().name = name;
}

Column& Column::schema(const Schema& schema)
{
    syntax().schema = schema;
    return *this;
}

Column& Column::table(const UnsafeStringView& table)
{
    syntax().table = table;
    return *this;
}

Column::~Column() = default;

Column Column::rowid()
{
    return Column("rowid");
}

OrderingTerm Column::asAscendingOrder()
{
    return OrderingTerm(*this).order(Order::ASC);
}

OrderingTerm Column::asDescendingOrder()
{
    return OrderingTerm(*this).order(Order::DESC);
}

Expression Column::asExpressionOperand() const
{
    return Expression(*this);
}

} // namespace WCDB