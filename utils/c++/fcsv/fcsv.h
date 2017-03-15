#ifndef _FCSV_H_
#define _FCSV_H_

struct fCSV;
struct fRow;

struct fCSV*    fcsv_create();
struct fCSV*    fcsv_open(char const* filename);
struct fCSV*    fcsv_read(char* data, unsigned int size);
void            fcsv_save(struct fCSV* csv, char const* filename);
void            fcsv_close(struct fCSV* csv);

struct fRow*    fcsv_first_row(struct fCSV* csv);
struct fRow*    fcsv_next_row(struct fRow* row);
struct fRow*    fcsv_insert_row(struct fRow* row);
struct fRow*    fcsv_insert_row_in_front(struct fRow* row);
void            fcsv_delete_row(struct fRow* row);

const char*     fcsv_get_field(struct fRow* row, unsigned int index);
void            fcsv_set_field(struct fRow* row, unsigned int index, const char* value);
void            fcsv_delete_field(struct fRow* row, unsigned int index);

unsigned int    fcsv_rows_count(struct fCSV* csv);
unsigned int    fcsv_fields_count(struct fRow* row);
void            fcsv_set_fields_count(struct fRow* row, unsigned int count);

#endif	// _FCSV_H_
