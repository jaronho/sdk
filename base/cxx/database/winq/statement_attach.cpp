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

#include "expr.hpp"
#include "statement_attach.hpp"

namespace WCDB {

Statement::Type StatementAttach::getStatementType() const
{
    return Statement::Type::Attach;
}

StatementAttach &StatementAttach::attach(const Expr &expr)
{
    m_description.append("ATTACH " + expr.getDescription());
    return *this;
}

StatementAttach &StatementAttach::attach(const Expr &expr,
                                         const std::string &database)
{
    m_description.append("ATTACH " + database + " " + expr.getDescription());
    return *this;
}

StatementAttach &StatementAttach::as(const std::string &schema)
{
    m_description.append(" AS " + schema);
    return *this;
}

} //namespace WCDB
