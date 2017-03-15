fCSV
===

A CSV file reader and writer library in C/C++.
***

## Installation

	sudo make install

## Example

```C++
#include <fcsv.h>
	
int main() {
	struct fCSV* csv = fcsv_create();
	struct fRow* row = 0;

	row = fcsv_first_row(csv);
	fcsv_set_fields_count(row, 3);
	fcsv_set_field(row, 0, "00");
	fcsv_set_field(row, 1, "01");
	fcsv_set_field(row, 2, "02");

	row = fcsv_insert_row(row);
	fcsv_set_fields_count(row, 3);
	fcsv_set_field(row, 0, "10");
	fcsv_set_field(row, 1, "11");
	fcsv_set_field(row, 2, "12");

	fcsv_save(csv, "test.csv");
	fcsv_close(csv);

	return 0;
}
```

```C++
#include <stdio.h>
#include <fcsv.h>

int main() {
	struct fCSV* csv = fcsv_open("test.csv");
	struct fRow* row = 0;

	row = fcsv_first_row(csv);
	for ( unsigned int i=0; i<fcsv_rows_count(csv); ++i ) {
		for ( unsigned int j=0; j<fcsv_fields_count(row); ++j ) {
			printf("r%df%d: %s\n", i, j, fcsv_get_field(row, j).c_str());
		}
		row = fcsv_next_row(row);
	}

	fcsv_close(csv);

	return 0;
}
```

## Licence

Copyright (C) 2014 funcman(hyq1986@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
