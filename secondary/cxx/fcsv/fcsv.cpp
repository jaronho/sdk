#include "fcsv.h"
#include <stdio.h>
#include <string.h>
#include "../mpool/mpool.h"
#include "../cvector/cvector.h"

struct fCSV;

struct fRow {
	cvector* fields;
	struct fRow* prev;
	struct fRow* next;
	struct fCSV* csv;
	fRow() : prev(NULL), next(NULL), csv(NULL) {
		fields = cvector_new();
	}
};

struct fCSV {
	unsigned int count;
	struct fRow* first;
	fCSV() : count(0), first(NULL) {}
};

enum SEPARATING_CHARACTER {
	SC_COMMA,
	SC_WIN_NEWLINE,
	SC_UNIX_NEWLINE,
	SC_EOF
};

static SEPARATING_CHARACTER s_cur_sc = SC_EOF;
static mpool_t* s_memory_pool = NULL;

static enum SEPARATING_CHARACTER get_sepchar_(char* str, unsigned int len, int* pos, int* last_pos) {
	for (unsigned int i = 0; i < len; ++i) {
		if (',' == str[i]) {
			*pos = i;
			*last_pos = i + 1;
			return SC_COMMA;
		} else if ('\r' == str[i] && i + 1 < len && '\n' == str[i+1]) {
			*pos = i;
			*last_pos = i + 2;
			return SC_WIN_NEWLINE;
		} else if ('\n' == str[i]) {
			*pos = i;
			*last_pos = i + 1;
			return SC_UNIX_NEWLINE;
		}
	}
	*pos = len;
	*last_pos = len;
	return SC_EOF;
}

static int get_comma_pos_(char* str, unsigned int len) {
	for (unsigned int i = 0; i < len; ++i) {
		if ('\"' == str[i]) {
			if (i + 1 < len && '\"' == str[i + 1]) {
				i++;
			} else {
				return i;
			}
		}
	}
	return 0;
}

static char* get_a_escaped_field_(char* str, unsigned int len, char** last_str) {
	int p = get_comma_pos_(str, len);
	int ret;
	char* s = (char*)mpool_calloc(s_memory_pool, p + 1, sizeof(char), &ret);
	strncpy(s, str, p);
	*(s + p) = '\0';
	int pos, last_pos;
	s_cur_sc = get_sepchar_(&str[p], len - p, &pos, &last_pos);
	*last_str = &str[p+last_pos];
	return s;
}

static char* get_a_non_escaped_field_(char* str, unsigned int len, char** last_str) {
	int pos, last_pos;
	s_cur_sc = get_sepchar_(str, len, &pos, &last_pos);
	int ret;
	char* s = (char*)mpool_calloc(s_memory_pool, pos + 1, sizeof(char), &ret);
	strncpy(s, str, pos);
	*(s + pos) = '\0';
	*last_str = &str[last_pos];
	return s;
}

static char* get_a_field_(char* str, unsigned int len, char** last_str) {
	if (len >= 1 && '\"' == str[0]) {
		return get_a_escaped_field_(str + 1, len - 1, last_str);
	} else {
		return get_a_non_escaped_field_(str, len, last_str);
	}
}

static void free_field(void* field) {
	mpool_free(s_memory_pool, field, sizeof((char*)field));
}

struct fCSV* fcsv_create() {
	if (NULL == s_memory_pool) {
		s_memory_pool = mpool_open(MPOOL_FLAG_BEST_FIT, 0, NULL, NULL);
	} else {
		mpool_clear(s_memory_pool);
	}
	struct fCSV* csv = new struct fCSV;
	fcsv_first_row(csv);
	return csv;
}

struct fCSV* fcsv_open(char const* filename) {
	struct fCSV* csv = NULL;
	FILE* fp = fopen(filename, "rb");
	if (fp) {
		char* csv_str = NULL;
		int size;
		fseek(fp, 0, SEEK_END);
		size = (int)ftell(fp);
		csv_str = new char[size];
		fseek(fp, 0, SEEK_SET);
		fread(csv_str, size, 1, fp);
		csv = fcsv_read(csv_str, size);
		delete[] csv_str;
		csv_str = NULL;
		fclose(fp);
	}
	return csv;
}

struct fCSV* fcsv_read(char* data, unsigned int size) {
	int ret;
	if (NULL == s_memory_pool) {
		s_memory_pool = mpool_open(MPOOL_FLAG_BEST_FIT, 0, NULL, &ret);
	} else {
		mpool_clear(s_memory_pool);
	}
	struct fCSV* csv = new struct fCSV;
	struct fRow* row = fcsv_first_row(csv);
	char* s1 = data;
	char* s2 = s1;
	unsigned int len = size;
	s_cur_sc = SC_EOF;
	do {
		cvector_push_back(row->fields, get_a_field_(s1, len, &s2));
		if (SC_WIN_NEWLINE == s_cur_sc || SC_UNIX_NEWLINE == s_cur_sc) {
			row = fcsv_insert_row(row);
		}
		len -= (s2 - s1);
		s1 = s2;
	} while(s_cur_sc != SC_EOF);
	return csv;
}

void fcsv_save(struct fCSV* csv, char const* filename) {
	FILE* fp = fopen(filename, "wb");
	if (fp) {
		struct fRow* row = fcsv_first_row(csv);
		while (row) {
			for (unsigned int i = 0; i < cvector_size(row->fields); ++i) {
				char* str = (char*)cvector_at(row->fields, i);
				fwrite(str, strlen(str), 1, fp);
				if (i + 1 < cvector_size(row->fields)) {
					fwrite(",", 1, 1, fp);
				}
			}
			row = fcsv_next_row(row);
			if (row) {
				fwrite("\r\n", 2, 1, fp);
			}
		}
		fclose(fp);
	}
}

void fcsv_close(struct fCSV* csv) {
	if (NULL == csv) {
		return;
	}
	while (csv->first) {
		fcsv_delete_row(csv->first);
	}
	delete csv;
	csv = NULL;
	if (s_memory_pool) {
		mpool_close(s_memory_pool);
		s_memory_pool = NULL;
	}
}

struct fRow* fcsv_first_row(struct fCSV* csv) {
	if (!csv->first) {
		csv->first = new struct fRow;
		csv->first->csv = csv;
		csv->count++;
	}
	return csv->first;
}

struct fRow* fcsv_next_row(struct fRow* row) {
	return row->next;
}

struct fRow* fcsv_insert_row(struct fRow* row) {
	struct fRow* new_row = new struct fRow;
	if (row->next) {
		row->next->prev = new_row;
		new_row->next = row->next;
	}
	row->next = new_row;
	new_row->prev = row;
	new_row->csv = row->csv;
	row->csv->count++;
	return new_row;
}

struct fRow* fcsv_insert_row_in_front(struct fRow* row) {
	if (row->prev) {
		return fcsv_insert_row(row->prev);
	}
	row->prev = new struct fRow;
	row->prev->next = row;
	row->prev->csv = row->csv;
	row->csv->count++;
	row->csv->first = row->prev;
	return row->prev;
}

void fcsv_delete_row(struct fRow* row) {
	if (row->prev && row->next) {
		row->prev->next = row->next;
		row->next->prev = row->prev;
	} else if (row->prev) {
		row->prev->next = NULL;
	} else if (row->next) {
		row->next->prev = NULL;
	}
	if (row->csv->first == row) {
		row->csv->first = row->next;
	}
	row->csv->count--;
	cvector_free(row->fields, free_field);
	delete row;
	row = NULL;
}

const char* fcsv_get_field(struct fRow* row, unsigned int index) {
	return (const char*)cvector_at(row->fields, index);
}

void fcsv_set_field(struct fRow* row, unsigned int index, const char* value) {
    if (index >= cvector_capacity(row->fields)) {
		cvector_reserve(row->fields, index + 1);
	}
	size_t value_len = strlen(value);
	char* s = (char*)mpool_calloc(s_memory_pool, value_len + 1, sizeof(char), NULL);
	memcpy(s, value, value_len);
	*(s + value_len) = '\0';
	cvector_set_at(row->fields, index, s);
}

void fcsv_delete_field(struct fRow* row, unsigned int index) {
	if (index >= cvector_size(row->fields)) {
		return;
	}
	cvector_erase(row->fields, index);
}

unsigned int fcsv_rows_count(struct fCSV* csv) {
    return csv->count;
}

unsigned int fcsv_fields_count(struct fRow* row) {
	return (unsigned int)cvector_size(row->fields);
}

void fcsv_set_fields_count(struct fRow* row, unsigned int count) {
	cvector_reserve(row->fields, count);
}
