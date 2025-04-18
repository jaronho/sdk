// Copyright (c) 2017-2020 Thomas Fussell
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

#include "../../../include/xlnt/utils/path.hpp"
#include "open_stream.hpp"

namespace xlnt {
namespace detail {

#ifdef _MSC_VER
void open_stream(std::ifstream &stream, const std::wstring &path)
{
    stream.open(path, std::ios::binary);
}

void open_stream(std::ofstream &stream, const std::wstring &path)
{
    stream.open(path, std::ios::binary);
}

void open_stream(std::ifstream &stream, const std::string &path)
{
    open_stream(stream, xlnt::path(path).wstring());
}

void open_stream(std::ofstream &stream, const std::string &path)
{
    open_stream(stream, xlnt::path(path).wstring());
}
#else
void open_stream(std::ifstream &stream, const std::string &path)
{
    stream.open(path, std::ios::binary);
}

void open_stream(std::ofstream &stream, const std::string &path)
{
    stream.open(path, std::ios::binary);
}
#endif

} // namespace detail
} // namespace xlnt
