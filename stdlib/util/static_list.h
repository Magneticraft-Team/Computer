//
// Created by cout970 on 2017-08-30.
//

#ifndef MAGNETICRAFTCOMPUTER_FIXED_ARRAY_H
#define MAGNETICRAFTCOMPUTER_FIXED_ARRAY_H

// List with constant size (doesn't use malloc) of arbitrary types

#define StaticList(name, type, size, equals)                                                                            \
typedef struct {                                                                                                        \
    int used;                                                                                                           \
    type data;                                                                                                          \
} type##_item;                                                                                                          \
                                                                                                                        \
typedef struct {                                                                                                        \
    int capacity;                                                                                                       \
    int used;                                                                                                           \
    type##_item items[size];                                                                                            \
} type##_list;                                                                                                          \
                                                                                                                        \
type##_list (name) = { size, 0, {} };                                                                                   \
                                                                                                                        \
type * name##_add(type *data) {                                                                                         \
    if ((name).used >= (name).capacity) return NULL;                                                                    \
    int index = -1;                                                                                                     \
    for (int i = 0; i < (name).capacity; ++i) {                                                                         \
        if (!(name).items[i].used) {                                                                                    \
            index = i;                                                                                                  \
            break;                                                                                                      \
        }                                                                                                               \
    }                                                                                                                   \
    if (index != -1) {                                                                                                  \
        (name).used++;                                                                                                  \
        (name).items[index].used = 1;                                                                                   \
        memcpy(&(name).items[index].data, data, sizeof(type));                                                          \
        return &(name).items[index].data;                                                                               \
    }                                                                                                                   \
    return NULL;                                                                                                        \
}                                                                                                                       \
                                                                                                                        \
int name##_remove(type *data) {                                                                                         \
    if (data == NULL) return 0;                                                                                         \
    if ((name).used <= 0) return 0;                                                                                     \
    for (int i = 0; i < (name).capacity; ++i) {                                                                         \
        if ((name).items[i].used && equals(&(name).items[i].data, data)) {                                              \
            (name).used--;                                                                                              \
            (name).items[i].used = 0;                                                                                   \
            return 1;                                                                                                   \
        }                                                                                                               \
    }                                                                                                                   \
    return 0;                                                                                                           \
}                                                                                                                       \
                                                                                                                        \
int name##_index_of(type *data){                                                                                        \
    int index = -1;                                                                                                     \
                                                                                                                        \
    for (int i = 0; i < (name).capacity; ++i) {                                                                         \
        if ((name).items[i].used && equals(&(name).items[i].data, data)) {                                              \
            index = i;                                                                                                  \
            break;                                                                                                      \
        }                                                                                                               \
    }                                                                                                                   \
    return index;                                                                                                       \
}                                                                                                                       \

#endif //MAGNETICRAFTCOMPUTER_FIXED_ARRAY_H
