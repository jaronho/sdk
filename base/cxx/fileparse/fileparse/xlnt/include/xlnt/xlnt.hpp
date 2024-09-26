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

#pragma once

#include "xlnt_config.hpp"

// cell
#include "cell/cell.hpp"
#include "cell/cell_reference.hpp"
#include "cell/cell_type.hpp"
#include "cell/comment.hpp"
#include "cell/hyperlink.hpp"
#include "cell/index_types.hpp"
#include "cell/rich_text.hpp"
#include "cell/rich_text_run.hpp"

// packaging
#include "packaging/manifest.hpp"
#include "packaging/relationship.hpp"
#include "packaging/uri.hpp"

// styles
#include "styles/alignment.hpp"
#include "styles/border.hpp"
#include "styles/color.hpp"
#include "styles/fill.hpp"
#include "styles/font.hpp"
#include "styles/format.hpp"
#include "styles/number_format.hpp"
#include "styles/protection.hpp"
#include "styles/style.hpp"

// utils
#include "utils/calendar.hpp"
#include "utils/date.hpp"
#include "utils/datetime.hpp"
#include "utils/exceptions.hpp"
#include "utils/path.hpp"
#include "utils/time.hpp"
#include "utils/timedelta.hpp"
#include "utils/variant.hpp"

// workbook
#include "workbook/document_security.hpp"
#include "workbook/external_book.hpp"
#include "workbook/metadata_property.hpp"
#include "workbook/named_range.hpp"
#include "workbook/streaming_workbook_reader.hpp"
#include "workbook/streaming_workbook_writer.hpp"
#include "workbook/theme.hpp"
#include "workbook/workbook.hpp"
#include "workbook/worksheet_iterator.hpp"

// worksheet
#include "worksheet/cell_iterator.hpp"
#include "worksheet/cell_vector.hpp"
#include "worksheet/column_properties.hpp"
#include "worksheet/header_footer.hpp"
#include "worksheet/major_order.hpp"
#include "worksheet/page_margins.hpp"
#include "worksheet/page_setup.hpp"
#include "worksheet/pane.hpp"
#include "worksheet/range.hpp"
#include "worksheet/range_iterator.hpp"
#include "worksheet/range_reference.hpp"
#include "worksheet/row_properties.hpp"
#include "worksheet/selection.hpp"
#include "worksheet/sheet_protection.hpp"
#include "worksheet/sheet_view.hpp"
#include "worksheet/worksheet.hpp"
