#ifndef INCLUDE_CONTAINER_OF_H
#define INCLUDE_CONTAINER_OF_H

/* Inspired via the Linux-kernel macro 'container_of' */
#define container_of(ptr, type, member) \
    ((type *) ((char*)(ptr) - offsetof(type, member)))

#endif
