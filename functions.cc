#include "functions.hh"

#include <immintrin.h>

void foreach_element__single_linked_list(Element *first, Callback callback)
{
    for (Element *element = first; element; element = element->next) {
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
        callback(*begin[i]);
        _mm_prefetch(begin[i + prefetch_distance], _MM_HINT_T0);
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
