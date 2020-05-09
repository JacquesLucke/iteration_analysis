#pragma once

struct alignas(64) Element {
    Element *next = nullptr;
    Element *prev = nullptr;
    int value = 0;
    char padding[64 - 3 * sizeof(void *)];
};

static_assert(sizeof(Element) == 64, "");
static_assert(alignof(Element) == 64, "");

using Callback = void (*)(Element &element);

void foreach_element__single_linked_list(Element *first, Callback callback);
void foreach_element__double_linked_list__unordered(Element *first,
                                                    Element *last,
                                                    Callback callback);
void foreach_element__pointer_array(Element **begin,
                                    int size,
                                    Callback callback);
void foreach_element__pointer_array__with_prefetching(Element **begin,
                                                      int size,
                                                      int prefetch_distance,
                                                      Callback callback);
void foreach_element__struct_array(Element *begin,
                                   int size,
                                   Callback callback);
