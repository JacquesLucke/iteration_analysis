#include "functions.hh"

#include <cassert>
#include <cstdint>
#include <immintrin.h>

#define PREFETCH(ptr) _mm_prefetch((const char *)ptr, _MM_HINT_T0)

void foreach_element__single_linked_list(Element *first, Callback callback)
{
    for (Element *element = first; element; element = element->next) {
        callback(*element);
    }
}

void foreach_element__single_linked_list__with_prefetching(Element *first,
                                                           Callback callback)
{
    for (Element *element = first; element; element = element->next) {
        PREFETCH(element->next_hint);
        callback(*element);
    }
}

void foreach_element__double_linked_list__unordered(Element *first,
                                                    Element *last,
                                                    Callback callback)
{
    if (first == nullptr) {
        return;
    }

    if (first == last) {
        callback(*first);
        return;
    }

    Element *front = first;
    Element *back = last;

    while (true) {
        callback(*front);
        callback(*back);

        if (front->next == back) {
            break;
        }
        else if (front->next == back->prev) {
            callback(*front->next);
        }
        else {
            front = front->next;
            back = back->prev;
        }
    }
}

void foreach_element__double_linked_list__unordered__with_prefetching(
    Element *first, Element *last, Callback callback)
{
    if (first == nullptr) {
        return;
    }

    if (first == last) {
        callback(*first);
        return;
    }

    Element *front = first;
    Element *back = last;

    while (true) {
        PREFETCH(front->next_hint);
        PREFETCH(back->prev_hint);

        callback(*front);
        callback(*back);

        if (front->next == back) {
            break;
        }
        else if (front->next == back->prev) {
            callback(*front->next);
        }
        else {
            front = front->next;
            back = back->prev;
        }
    }
}

void foreach_element__pointer_array(Element **begin,
                                    int size,
                                    Callback callback)
{
    for (int i = 0; i < size; i++) {
        callback(*begin[i]);
    }
}

void foreach_element__pointer_array__with_prefetching(Element **begin,
                                                      int size,
                                                      int prefetch_distance,
                                                      Callback callback)
{
    for (int i = 0; i < size - prefetch_distance; i++) {
        PREFETCH(begin[i + prefetch_distance]);
        callback(*begin[i]);
    }

    for (int i = size - prefetch_distance; i < size; i++) {
        callback(*begin[i]);
    }
}

void foreach_element__struct_array(Element *begin, int size, Callback callback)
{
    for (int i = 0; i < size; i++) {
        callback(begin[i]);
    }
}

void foreach_element__struct_array__backwards(Element *begin,
                                              int size,
                                              Callback callback)
{
    for (int i = size; i--;) {
        callback(begin[i]);
    }
}
