#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <dictionary.h>

typedef struct entry entry;
struct entry
{
    entry* next;
    char* name;
    void* value;
};


#if ARCH!=ARCH_XMEGA
#   define HASHSIZE 13
#else
#   define HASHSIZE 101
#endif

struct dictionary {
  entry *hashtab[HASHSIZE];
};

static unsigned int
hash(char *name)
{
    unsigned int hashval;
    for (hashval = 0; *name != '\0'; name++) {
      hashval = *name + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

static entry* _get(dictionary* d, char *name)
{
  // printf("get %p %s\n", d, name);
  entry* entry;
  for (entry = d->hashtab[hash(name)]; entry != NULL; entry = entry->next) {
    if (strcmp(name, entry->name) == 0) {
      // printf("\tgot %p\n", entry);
      return entry;
    }
  }
  // puts("not\n");
  return NULL;
}

void* dictionary_get(dictionary* d, char* name)
{
  entry* entry = _get(d, name);
  
  if (entry) {
    return entry->value;
  }

  return NULL;
}

bool dictionary_has(dictionary* d, char *name)
{
  entry* entry = _get(d, name);

  if (entry) {
    return true;
  }

  return false;
}

void
dictionary_put(dictionary* d, char* name, void* value)
{
    // printf("put %p %s\n", d, name);
    entry* element;
    unsigned int hashval;

    element = _get(d, name);

    if (element == NULL) {
        element = (entry*) malloc(sizeof(*element));
        if (element == NULL || (element->name = strdup(name)) == NULL) {
          return;
        }
        hashval = hash(name);
        element->next = d->hashtab[hashval];
        d->hashtab[hashval] = element;
    }
    // printf("entry %p\n", element);
    element->value = value;
}

void*
dictionary_del(dictionary* d, char* name)
{
  entry* element = _get(d, name);

  if (element != NULL) {
    entry* previous = NULL;
    for(entry* entry = d->hashtab[hash(name)]; entry != NULL; entry = entry->next) {
      if (strcmp(name, entry->name) == 0) {
        if (previous && entry->next) {
          previous->next = entry->next;
        }
        if (!previous && !entry->next) {
          d->hashtab[hash(name)] = NULL;
        }
      }
      previous = entry;
    }
    void *value = element->value;
    free(element); 
    return value;
  }
  
  return NULL;
}

void
dictionary_each(dictionary* d, dictionary_each_cb cb, void* context)
{

  if (!cb) {
    return;
  }

  for(size_t i=0; i < HASHSIZE; i++) {
    entry* entry = d->hashtab[i];
    while(entry) {
      cb(entry->name, entry->value, context);
      entry = entry->next;
    }
  }
}

dictionary*
dictionary_new()
{
  // printf("dictionary size: %ld\n", sizeof(dictionary));
  dictionary* d = malloc(sizeof(dictionary));
  if (d == NULL) {
    return NULL;
  }
  for(size_t i=0; i < HASHSIZE; i++) {
    d->hashtab[i] = NULL;
  }
  return d;
}

void
dictionary_destroy(dictionary* d, dictionary_each_cb free_cb)
{
  dictionary_each(d, free_cb, NULL);
  free(d);
}
