#include "functions.hh"

#include <cassert>
#include <cstdint>
#include <immintrin.h>
#include <iostream>
#include <random>
#include <stack>
#include <vector>

#define PREFETCH(ptr) _mm_prefetch((const char *)ptr, _MM_HINT_T0)

void foreach_element__single_linked_list(Element *first, Callback callback)
{
    for (Element *element = first; element; element = element->next) {
        callback(element);
    }
}

void foreach_element__single_linked_list__with_prefetching(Element *first,
                                                           Callback callback)
{
    for (Element *element = first; element; element = element->next) {
        PREFETCH(element->next_hint);
        callback(element);
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
        callback(first);
        return;
    }

    Element *front = first;
    Element *back = last;

    while (true) {
        callback(front);
        callback(back);

        if (front->next == back) {
            break;
        }
        else if (front->next == back->prev) {
            callback(front->next);
            break;
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
        callback(first);
        return;
    }

    Element *front = first;
    Element *back = last;

    while (true) {
        PREFETCH(front->next_hint);
        PREFETCH(back->prev_hint);

        callback(front);
        callback(back);

        if (front->next == back) {
            break;
        }
        else if (front->next == back->prev) {
            callback(front->next);
            break;
        }
        else {
            front = front->next;
            back = back->prev;
        }
    }
}

void foreach_element__double_linked_list__ordered__std_stack(Element *first,
                                                             Element *last,
                                                             Callback callback)
{
    if (first == nullptr) {
        return;
    }

    std::stack<Element *> elements;

    Element *front = first;
    Element *back = last;

    while (true) {
        callback(front);

        if (front == back) {
            break;
        }
        if (front->next == back) {
            elements.push(back);
            break;
        }

        elements.push(back);
        front = front->next;
        back = back->prev;
    }

    while (!elements.empty()) {
        callback(elements.top());
        elements.pop();
    }
}

void foreach_element__double_linked_list__ordered__std_vector(
    Element *first, Element *last, Callback callback)
{
    if (first == nullptr) {
        return;
    }

    std::vector<Element *> elements;

    Element *front = first;
    Element *back = last;

    while (true) {
        callback(front);

        if (front == back) {
            break;
        }
        if (front->next == back) {
            elements.push_back(back);
            break;
        }

        elements.push_back(back);
        front = front->next;
        back = back->prev;
    }

    for (int i = elements.size(); i--;) {
        callback(elements[i]);
    }
}

class MyStack {
  private:
    Element **m_arrays[30];
    Element **m_current_array;
    int m_current_array_index;
    int m_current_size;
    int m_current_capacity;

    static int capacity_of_array(int index)
    {
        return 1 << (index + 7);
    }

  public:
    MyStack()
    {
        m_current_array_index = 0;
        m_current_size = 0;
        m_current_capacity = this->capacity_of_array(m_current_array_index);
        m_arrays[0] = (Element **)malloc(sizeof(Element *) *
                                         m_current_capacity);
        m_current_array = m_arrays[0];
    }

    ~MyStack()
    {
        for (int i = 0; i <= m_current_array_index; i++) {
            free(m_arrays[i]);
        }
    }

    void push(Element *element)
    {
        if (m_current_size == m_current_capacity) {
            m_current_array_index++;
            m_current_capacity = this->capacity_of_array(
                m_current_array_index);
            m_arrays[m_current_array_index] = (Element **)malloc(
                sizeof(Element *) * m_current_capacity);
            m_current_array = m_arrays[m_current_array_index];
            m_current_size = 0;
        }
        m_current_array[m_current_size] = element;
        m_current_size++;
    }

    void call_in_reverse(Callback callback)
    {
        {
            Element **array = m_current_array;
            for (int i = m_current_size; i--;) {
                callback(array[i]);
            }
        }
        if (m_current_array_index == 0) {
            return;
        }
        for (int array_index = m_current_array_index; array_index--;) {
            Element **array = m_arrays[array_index];
            for (int i = this->capacity_of_array(array_index); i--;) {
                callback(array[i]);
            }
        }
    }
};

void foreach_element__double_linked_list__ordered__custom(Element *first,
                                                          Element *last,
                                                          Callback callback)
{
    if (first == nullptr) {
        return;
    }

    MyStack elements;

    Element *front = first;
    Element *back = last;

    while (true) {
        callback(front);

        if (front == back) {
            break;
        }
        if (front->next == back) {
            elements.push(back);
            break;
        }

        elements.push(back);
        front = front->next;
        back = back->prev;
    }

    elements.call_in_reverse(callback);
}

void foreach_element__pointer_array(Element **begin,
                                    int size,
                                    Callback callback)
{
    for (int i = 0; i < size; i++) {
        callback(begin[i]);
    }
}

void foreach_element__pointer_array__with_prefetching(Element **begin,
                                                      int size,
                                                      int prefetch_distance,
                                                      Callback callback)
{
    for (int i = 0; i < size - prefetch_distance; i++) {
        PREFETCH(begin[i + prefetch_distance]);
        callback(begin[i]);
    }

    for (int i = std::max(size - prefetch_distance, 0); i < size; i++) {
        callback(begin[i]);
    }
}

void foreach_element__struct_array(Element *begin, int size, Callback callback)
{
    for (int i = 0; i < size; i++) {
        callback(begin + i);
    }
}

void foreach_element__struct_array__backwards(Element *begin,
                                              int size,
                                              Callback callback)
{
    for (int i = size; i--;) {
        callback(begin + i);
    }
}

void foreach_element__struct_array__chunked(Element *begin,
                                            int size,
                                            Callback callback)
{
    assert(size % 8 == 0);
    for (int i = 0; i + 8 <= size; i += 8) {
        Element *current = begin + i;
        callback(current);
        callback(current + 1);
        callback(current + 2);
        callback(current + 3);
        callback(current + 4);
        callback(current + 5);
        callback(current + 6);
        callback(current + 7);
    }
}

unsigned int xorshift32()
{
    static unsigned int x = 345362423;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

void clobber_cache()
{
    unsigned int buffer_size = (1 << 28);
    unsigned int mask = buffer_size - 1;
    char *buffer = new char[buffer_size]();

    for (int i = 0; i < 1000000; i++) {
        int index = mask & xorshift32();
        buffer[index]++;
    }

    delete[] buffer;
}
