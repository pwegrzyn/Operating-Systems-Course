/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#ifndef IOFUNCS_H
#define IOFUNCS_H

#include <stdlib.h>

int generate(size_t record_size, int no_records, const char *output_file);
int copy_sys(const char *in_file, const char *out_file, size_t buffer_size, int no_records);
int copy_lib(const char *in_file, const char *out_file, size_t buffer_size, int no_records);
int sort_sys(const char *input_file, int no_records, size_t record_size);
int sort_lib(const char *input_file, int no_records, size_t record_size);

#endif