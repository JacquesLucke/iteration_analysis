#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#include "functions.hh"

static void update_linked_list_pointers(std::vector<Element *> &elements)
{
    int size = elements.size();
    for (int i = 1; i < size - 1; i++) {
        elements[i]->next = elements[i + 1];
        elements[i]->prev = elements[i - 1];
    }

    elements[0]->next = elements[1];
    elements[size - 1]->prev = elements[size - 2];
}

class Benchmark {
  private:
    std::unordered_map<std::string, std::vector<std::chrono::nanoseconds>>
        m_results;

  public:
    void add_result(std::string name, std::chrono::nanoseconds measurement)
    {
        m_results[name].push_back(measurement);
    }

    void print()
    {
        for (auto &result : m_results) {
            std::cout << result.first << "\n";
            for (std::chrono::nanoseconds measurement : result.second) {
                std::cout << "  " << (measurement.count() / 1.0e6) << " ms\n";
            }
        }
    }

    class Timer {
      private:
        Benchmark &m_benchmark;
        std::string m_name;
        std::chrono::steady_clock::time_point m_start_time;

      public:
        Timer(Benchmark &benchmark, std::string name)
            : m_benchmark(benchmark), m_name(name)
        {
            m_start_time = std::chrono::steady_clock::now();
        }

        ~Timer()
        {
            std::chrono::steady_clock::time_point end_time =
                std::chrono::steady_clock::now();
            std::chrono::nanoseconds duration = end_time - m_start_time;
            m_benchmark.add_result(m_name, duration);
        }
    };
};

void run_benchmarks(std::vector<Element> &elements,
                    std::vector<Element *> &sorted_element_pointers,
                    std::vector<Element *> &randomized_element_pointers)
{
    Benchmark benchmark;

#define SCOPED_TIMER(name) Benchmark::Timer timer(benchmark, (name))

    int size = elements.size();
    auto callback = [](Element &element) { element.value++; };

    for (int i = 0; i < 3; i++) {
        std::cout << "Iteration: " << (i + 1) << "\n";
        update_linked_list_pointers(randomized_element_pointers);
        {
            SCOPED_TIMER("Single Linked List");
            foreach_element__single_linked_list(randomized_element_pointers[0],
                                                callback);
        }
        {
            SCOPED_TIMER("Double Linked List");
            foreach_element__double_linked_list__unordered(
                randomized_element_pointers[0],
                randomized_element_pointers[size - 1],
                callback);
        }
        {
            SCOPED_TIMER("Pointer Array");
            foreach_element__pointer_array(
                randomized_element_pointers.data(), size, callback);
        }
        {
            SCOPED_TIMER("Struct Array");
            foreach_element__struct_array(elements.data(), size, callback);
        }
    }

    benchmark.print();
}

int main(int argc, char const *argv[])
{
    const int amount = 1'000'000;

    std::vector<Element> elements(amount);
    std::vector<Element *> sorted_element_pointers;

    for (Element &element : elements) {
        sorted_element_pointers.push_back(&element);
    }

    std::vector<Element *> randomized_element_pointers =
        sorted_element_pointers;

    std::shuffle(randomized_element_pointers.begin(),
                 randomized_element_pointers.end(),
                 std::default_random_engine());

    run_benchmarks(
        elements, sorted_element_pointers, randomized_element_pointers);

    return 0;
}
