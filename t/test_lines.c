#include "test.h"

#include <lines.h>

#include <stdio.h>

static char __memory[1024];
static size_t __memory_size = sizeof(__memory);

int lines_setup(void **state)
{
  lines_init(__memory, __memory_size);
  return 0;
}

int lines_teardown(void **state)
{
  return 0;
}

static void out(size_t number, char* contents)
{
  static int calls = 0;
  
  printf("(%ld) '%s'\n", number, contents);
  
  calls++;

  assert_int_equal( number, 10 * calls );
}

void test_lines(void **state)
{
  assert_true
  (
    lines_insert(10, "XXX")
  );

  line* l10 = lines_get_by_number(10);
  
  assert_int_equal
  (
    l10->number, 10
  );  
  assert_string_equal
  (
    l10->contents, "XXX"
  ); 

  assert_true
  (
    lines_store(20, "YYY")
  );

  line* l20 = lines_get_by_number(20);
  
  assert_int_equal
  (
    l20->number, 20
  );  
  assert_string_equal
  (
    l20->contents, "YYY"
  ); 
 
  lines_list(out); 
}
