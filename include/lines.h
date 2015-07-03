#ifndef __LINES_H__
#define __LINES_H__

#include <stdbool.h>

typedef struct line line;

struct line
{
  size_t  number;
  bool    deleted;
  line*   next_line;
  char    contents[];
};

void lines_init(char *memory, size_t memory_size);

void lines_reset(void);

line* lines_next(void);

line* lines_get_by_number(size_t line_number);

line* lines_current(void);

line* lines_previous(line* current);

line* lines_last(void);

void lines_set_by_number(size_t line_number);

bool lines_exists(size_t number);

bool lines_insert(size_t number, char *contents);

bool lines_delete(size_t number);

bool lines_replace(size_t number, char *contents);

bool lines_store(size_t number, char* contents );

typedef void (*lines_list_cb)(size_t number, char* contents);

void lines_list(lines_list_cb out);

#endif // __LINES_H__
