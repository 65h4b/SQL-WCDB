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

#include <WCDB/Syntax.h>
#include <WCDB/SyntaxAssertion.hpp>

namespace WCDB {

namespace Syntax {

DeleteSTMT::~DeleteSTMT() = default;

bool DeleteSTMT::isValid() const
{
    return table.isValid();
}

#pragma mark - Identifier
Identifier::Type DeleteSTMT::getType() const
{
    return type;
}

bool DeleteSTMT::describle(std::ostringstream& stream) const
{
    return describle(stream, false);
}

bool DeleteSTMT::describle(std::ostringstream& stream, bool skipSchema) const
{
    if (!commonTableExpressions.empty()) {
        stream << "WITH ";
        if (recursive) {
            stream << "RECURSIVE ";
        }
        stream << commonTableExpressions << space;
    }
    stream << "DELETE FROM ";
    if (!table.describle(stream, skipSchema)) {
        return false;
    }
    if (condition.isValid()) {
        stream << " WHERE " << condition;
    }
    if (!orderingTerms.empty()) {
        stream << " ORDER BY " << orderingTerms;
    }
    if (limit.isValid()) {
        stream << " LIMIT " << limit;
        switch (limitParameterType) {
        case LimitParameterType::NotSet:
            break;
        case LimitParameterType::Offset:
            stream << " OFFSET " << limitParameter;
            break;
        case LimitParameterType::End:
            stream << ", " << limitParameter;
            break;
        }
    }
    return true;
}

void DeleteSTMT::iterate(const Iterator& iterator, bool& stop)
{
    Identifier::iterate(iterator, stop);
    listIterate(commonTableExpressions, iterator, stop);
    recursiveIterate(table, iterator, stop);
    if (condition.isValid()) {
        recursiveIterate(condition, iterator, stop);
    }
    listIterate(orderingTerms, iterator, stop);
    if (limit.isValid()) {
        recursiveIterate(limit, iterator, stop);
        switch (limitParameterType) {
        case LimitParameterType::NotSet:
            break;
        case LimitParameterType::Offset:
        case LimitParameterType::End:
            recursiveIterate(limitParameter, iterator, stop);
            break;
        }
    }
}

} // namespace Syntax

} // namespace WCDB