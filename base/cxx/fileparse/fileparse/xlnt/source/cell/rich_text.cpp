// Copyright (c) 2014-2020 Thomas Fussell
// Copyright (c) 2010-2015 openpyxl
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
#include <numeric>

#include "../../include/xlnt/cell/rich_text.hpp"
#include "../../include/xlnt/cell/rich_text_run.hpp"

namespace {
bool has_trailing_whitespace(const std::string &s)
{
    return !s.empty() && (s.front() == ' ' || s.back() == ' ');
};
} // namespace

namespace xlnt {

rich_text::rich_text(const std::string &plain_text)
    : rich_text(rich_text_run{plain_text, optional<font>(), has_trailing_whitespace(plain_text)})
{
}

rich_text::rich_text(const std::string &plain_text, const class font &text_font)
    : rich_text(rich_text_run{plain_text, optional<font>(text_font), has_trailing_whitespace(plain_text)})
{
}

rich_text::rich_text(const rich_text &other)
{
    *this = other;
}

rich_text &rich_text::operator=(const rich_text &rhs)
{
    clear();
    runs_ = rhs.runs_;
    phonetic_runs_ = rhs.phonetic_runs_;
    phonetic_properties_ = rhs.phonetic_properties_;
    return *this;
}

rich_text::rich_text(const rich_text_run &single_run)
{
    add_run(single_run);
}

void rich_text::clear()
{
    runs_.clear();
    phonetic_runs_.clear();
    phonetic_properties_.clear();
}

void rich_text::plain_text(const std::string &s, bool preserve_space = false)
{
    clear();
    add_run(rich_text_run{s, {}, preserve_space});
}

std::string rich_text::plain_text() const
{
    if (runs_.size() == 1)
    {
        return runs_.begin()->first;
    }

    return std::accumulate(runs_.begin(), runs_.end(), std::string(),
        [](const std::string &a, const rich_text_run &run) { return a + run.first; });
}

std::vector<rich_text_run> rich_text::runs() const
{
    return runs_;
}

void rich_text::runs(const std::vector<rich_text_run> &new_runs)
{
    runs_ = new_runs;
}

void rich_text::add_run(const rich_text_run &t)
{
    runs_.push_back(t);
}

std::vector<phonetic_run> rich_text::phonetic_runs() const
{
    return phonetic_runs_;
}

void rich_text::phonetic_runs(const std::vector<phonetic_run> &new_phonetic_runs)
{
    phonetic_runs_ = new_phonetic_runs;
}

void rich_text::add_phonetic_run(const phonetic_run &r)
{
    phonetic_runs_.push_back(r);
}

bool rich_text::has_phonetic_properties() const
{
    return phonetic_properties_.is_set();
}

const phonetic_pr &rich_text::phonetic_properties() const
{
    return phonetic_properties_.get();
}

void rich_text::phonetic_properties(const phonetic_pr &phonetic_props)
{
    phonetic_properties_.set(phonetic_props);
}

bool rich_text::operator==(const rich_text &rhs) const
{
    if (runs_.size() != rhs.runs_.size()) return false;

    for (std::size_t i = 0; i < runs_.size(); i++)
    {
        if (runs_[i] != rhs.runs_[i]) return false;
    }

    if (phonetic_runs_.size() != rhs.phonetic_runs_.size()) return false;

    for (std::size_t i = 0; i < phonetic_runs_.size(); i++)
    {
        if (phonetic_runs_[i] != rhs.phonetic_runs_[i]) return false;
    }

    if (phonetic_properties_ != rhs.phonetic_properties_) return false;

    return true;
}

bool rich_text::operator==(const std::string &rhs) const
{
    return *this == rich_text(rhs);
}

bool rich_text::operator!=(const rich_text &rhs) const
{
    return !(*this == rhs);
}

bool rich_text::operator!=(const std::string &rhs) const
{
    return !(*this == rhs);
}

} // namespace xlnt
