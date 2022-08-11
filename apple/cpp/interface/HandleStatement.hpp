//
// Created by qiuwenchen on 2022/8/3.
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
#include <WCDB/Statement.hpp>
#include <WCDB/StatementOperation.hpp>

namespace WCDB {

class InnerHandleStatement;
class Handle;

class HandleStatement : public StatementOperation {
    friend class Handle;

public:
    HandleStatement(HandleStatement &&other);
    ~HandleStatement() override final;
    StringView m_tag;

protected:
    HandleStatement() = delete;
    HandleStatement(const HandleStatement &) = delete;
    HandleStatement &operator=(const HandleStatement &) = delete;
    HandleStatement(InnerHandleStatement *handleStatement);

    InnerHandleStatement *getInnerHandleStatement() override final;

private:
    InnerHandleStatement *m_innerHandleStatement;
};

} //namespace WCDB