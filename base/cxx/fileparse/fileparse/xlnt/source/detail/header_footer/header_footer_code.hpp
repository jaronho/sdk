// Copyright (c) 2014-2020 Thomas Fussell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, WRISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE
//
// @license: http://www.opensource.org/licenses/mit-license.php
// @author: see AUTHORS file

#include <array>
#include <string>

#include "../../../include/xlnt/cell/rich_text.hpp"
#include "../../../include/xlnt/utils/optional.hpp"
#include "../../../include/xlnt/worksheet/header_footer.hpp"
#include "../../../include/xlnt/utils/numeric.hpp"

namespace xlnt {
namespace detail {

std::array<xlnt::optional<xlnt::rich_text>, 3> decode_header_footer(const std::string &hf_string, const number_serialiser &serialiser);
std::string encode_header_footer(const rich_text &t, header_footer::location where, const number_serialiser& serialiser);

} // namespace detail
} // namespace xlnt
