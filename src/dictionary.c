#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct entry entry;
struct entry
{
    entry* next;
    char* name;
    void* value;
};

#define HASHSIZE 101
static entry* hashtab[HASHSIZE];

typedef struct {
  entry *hashtab[HASHSIZE];
} dictionary;

typedef void (*dictionary_each_cb)(char* name, void* value);

dictionary* dictionary_new(void) { return NULL; }
void dictionary_destroy(dictionary* d, dictionary_each_cb cb) {}
void dictionary_put(dictionary* d, char* name, void* value) {}
bool dictionary_has(dictionary* d, char* name) { return false; }
void* dictionary_get(dictionary* d, char* name) { return NULL; }
void dictionary_each(dictionary* d, dictionary_each_cb cb) {}

static unsigned int
hash(char *name)
{
    unsigned int hashval;
    for (hashval = 0; *name != '\0'; name++) {
      hashval = *name + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

static entry* _get(char *s)
{
  entry* entry;
  for (entry = hashtab[hash(s)]; entry != NULL; entry = entry->next) {
    if (strcmp(s, entry->name) == 0) {
      return entry;
    }
  }
  return NULL;
}

void* get(char* name)
{
  entry* entry = _get(name);
  
  if (entry) {
    return entry->value;
  }

  return NULL;
}

bool has(char *name)
{
  entry* entry = _get(name);

  if (entry) {
    return true;
  }

  return false;
}

void
put(char* name, void* value)
{
    entry* element;
    unsigned int hashval;

    element = _get(name);

    if (element == NULL) {
        element = (entry*) malloc(sizeof(*element));
        if (element == NULL || (element->name = strdup(name)) == NULL) {
          return;
        }
        hashval = hash(name);
        element->next = hashtab[hashval];
        hashtab[hashval] = element;
    }

    element->value = value;
}
