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

#ifndef column_index_hpp
#define column_index_hpp

#include "declare.hpp"
#include "describable.hpp"
#include "order_term.hpp"

namespace WCDB {

class ColumnIndex : public Describable {
public:
    ColumnIndex(const Column &column, OrderTerm term = OrderTerm::NotSet);
    ColumnIndex(const Expr &expr, OrderTerm term = OrderTerm::NotSet);

    operator ColumnIndexList() const;
};

} //namespace WCDB

#endif /* column_index_hpp */
