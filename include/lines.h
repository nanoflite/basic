#ifndef __LINES_H__
#define __LINES_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct line line;

struct line
{
  uint16_t  number;
  uint8_t   length;
  char      contents;
};

void lines_init(char *memory, size_t memory_size);
// 
// void lines_reset(void);
// 
// line* lines_next(void);
// 
// line* lines_get_by_number(size_t line_number);
// 
// line* lines_current(void);
// 
// line* lines_previous(line* current);
// 
// line* lines_last(void);
// 
// void lines_set_by_number(size_t line_number);
// 
// bool lines_exists(size_t number);
// 
// bool lines_insert(size_t number, char *contents);
// 
bool lines_delete(uint16_t number);
// 
// bool lines_replace(size_t number, char *contents);
// 
bool lines_store(uint16_t number, char* contents );
 
typedef void (*lines_list_cb)(size_t number, char* contents);
 
void lines_list(lines_list_cb out);

void lines_clear(void);

char* lines_get(uint16_t number);

uint16_t lines_next(uint16_t number);

#endif // __LINES_H__
