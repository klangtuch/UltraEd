#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <malloc.h>
#include "n64sdk\ultra\GCC\MIPSE\INCLUDE\STRING.H"

#define HASHSIZE 100

struct nlist
{
    struct nlist *next;
    char *name;
    unsigned int gameObjectIndex;
};

static struct nlist *hashtable[HASHSIZE];

unsigned hash(const char *s)
{
    unsigned int hashval;
    for (hashval = 0; *s != '\0'; s++)
    {
        hashval = *s + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

struct nlist *lookup(const char *s)
{
    struct nlist *np;
    for (np = hashtable[hash(s)]; np != NULL; np = np->next)
    {
        if (strcmp(s, np->name) == 0)
        {
            return np;
        }
    }
    return NULL;
}

struct nlist *insert(const char *name, unsigned int index)
{
    struct nlist *np;
    if ((np = lookup(name)) == NULL)
    {
        np = (struct nlist*)malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL) return NULL;
        unsigned int hashval = hash(name);
        np->next = hashtable[hashval];
        hashtable[hashval] = np;
    }

    np->gameObjectIndex = index;
    return np;
}

#endif
