#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "functions.hh"

static void update_linked_list_pointers(std::vector<Element *> &elements,
                                        int prefetch_hint_distance)
{
    int size = elements.size();
    for (int i = 1; i < size - 1; i++) {
        elements[i]->next = elements[i + 1];
        elements[i]->prev = elements[i - 1];
    }

    elements[0]->prev = nullptr;
    elements[0]->next = elements[1];
    elements[size - 1]->prev = elements[size - 2];
    elements[size - 1]->next = nullptr;

    for (int i = 0; i < size - prefetch_hint_distance; i++) {
        Element *a = elements[i];
        Element *b = elements[i + prefetch_hint_distance];
        a->next_hint = (const char *)b;
        b->prev_hint = (const char *)a;
    }
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
        std::vector<std::pair<long long, std::string>> averages;

        for (auto &result : m_results) {
            long long sum = 0;
            for (std::chrono::nanoseconds measurement : result.second) {
                sum += measurement.count();
            }
            averages.emplace_back(
                std::make_pair(sum / result.second.size(), result.first));
        }
        std::sort(averages.begin(), averages.end());

        for (auto result : averages) {
            std::cout << std::left << std::setw(70) << result.second
                      << std::setprecision(5) << (result.first / 1.0e6)
                      << " ms\n";
        }
    }

    int amount()
    {
        return m_results.size();
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

#define SCOPED_BENCHMARK(name) \
    clobber_cache(); \
    Benchmark::Timer timer(benchmark, (name))

    int size = elements.size();
    auto callback = [](Element *element) { element->value++; };

    std::vector<int> prefetch_distances = {0, 1, 2, 4, 8, 16, 32, 64};

    int iterations = 10;

    for (int i = 0; i < iterations; i++) {
        std::cout << "Iteration: " << (i + 1) << "/" << iterations << "\n";
        {
            update_linked_list_pointers(sorted_element_pointers, 0);
            SCOPED_BENCHMARK("Sorted Single Linked List");
            foreach_element__single_linked_list(sorted_element_pointers[0],
                                                callback);
        }
        {
            update_linked_list_pointers(sorted_element_pointers, 0);
            SCOPED_BENCHMARK("Sorted Double Linked List");
            foreach_element__double_linked_list__unordered(
                sorted_element_pointers[0],
                sorted_element_pointers[size - 1],
                callback);
        }
        {
            update_linked_list_pointers(randomized_element_pointers, 0);
            SCOPED_BENCHMARK("Randomized Single Linked List");
            foreach_element__single_linked_list(randomized_element_pointers[0],
                                                callback);
        }
        {
            for (int prefetch_distance : prefetch_distances) {

                update_linked_list_pointers(randomized_element_pointers,
                                            prefetch_distance);
                SCOPED_BENCHMARK(
                    "Randomized Single Linked List with Prefetching "
                    "(distance=" +
                    std::to_string(prefetch_distance) + ")");
                foreach_element__single_linked_list__with_prefetching(
                    randomized_element_pointers[0], callback);
            }
        }
        {
            update_linked_list_pointers(randomized_element_pointers, 0);
            SCOPED_BENCHMARK("Randomized Double Linked List");
            foreach_element__double_linked_list__unordered(
                randomized_element_pointers[0],
                randomized_element_pointers[size - 1],
                callback);
        }
        {
            for (int prefetch_distance : prefetch_distances) {
                update_linked_list_pointers(randomized_element_pointers,
                                            prefetch_distance);
                SCOPED_BENCHMARK(
                    "Randomized Double Linked List with Prefetching "
                    "(distance=" +
                    std::to_string(prefetch_distance) + ")");
                foreach_element__double_linked_list__unordered__with_prefetching(
                    randomized_element_pointers[0],
                    randomized_element_pointers[size - 1],
                    callback);
            }
        }
        {
            update_linked_list_pointers(randomized_element_pointers, 0);
            SCOPED_BENCHMARK("Randomized Double Linked List - std::stack");
            foreach_element__double_linked_list__ordered__std_stack(
                randomized_element_pointers[0],
                randomized_element_pointers[size - 1],
                callback);
        }
        {
            update_linked_list_pointers(sorted_element_pointers, 0);
            SCOPED_BENCHMARK("Sorted Double Linked List - std::stack");
            foreach_element__double_linked_list__ordered__std_stack(
                sorted_element_pointers[0],
                sorted_element_pointers[size - 1],
                callback);
        }
        {
            update_linked_list_pointers(randomized_element_pointers, 0);
            SCOPED_BENCHMARK("Randomized Double Linked List - std::vector");
            foreach_element__double_linked_list__ordered__std_vector(
                randomized_element_pointers[0],
                randomized_element_pointers[size - 1],
                callback);
        }
        {
            update_linked_list_pointers(sorted_element_pointers, 0);
            SCOPED_BENCHMARK("Sorted Double Linked List - std::vector");
            foreach_element__double_linked_list__ordered__std_vector(
                sorted_element_pointers[0],
                sorted_element_pointers[size - 1],
                callback);
        }
        {
            update_linked_list_pointers(randomized_element_pointers, 0);
            SCOPED_BENCHMARK("Randomized Double Linked List - custom");
            foreach_element__double_linked_list__ordered__custom(
                randomized_element_pointers[0],
                randomized_element_pointers[size - 1],
                callback);
        }
        {
            update_linked_list_pointers(sorted_element_pointers, 0);
            SCOPED_BENCHMARK("Sorted Double Linked List - custom");
            foreach_element__double_linked_list__ordered__custom(
                sorted_element_pointers[0],
                sorted_element_pointers[size - 1],
                callback);
        }
        {
            SCOPED_BENCHMARK("Randomized Pointer Array");
            foreach_element__pointer_array(
                randomized_element_pointers.data(), size, callback);
        }
        {
            SCOPED_BENCHMARK("Sorted Pointer Array");
            foreach_element__pointer_array(
                sorted_element_pointers.data(), size, callback);
        }
        {
            for (int prefetch_distance : prefetch_distances) {
                SCOPED_BENCHMARK(
                    "Randomized Pointer Array with Prefetching (distance=" +
                    std::to_string(prefetch_distance) + ")");
                foreach_element__pointer_array__with_prefetching(
                    randomized_element_pointers.data(),
                    size,
                    prefetch_distance,
                    callback);
            }
        }
        {
            for (int prefetch_distance : prefetch_distances) {
                SCOPED_BENCHMARK(
                    "Sorted Pointer Array with Prefetching (distance=" +
                    std::to_string(prefetch_distance) + ")");
                foreach_element__pointer_array__with_prefetching(
                    sorted_element_pointers.data(),
                    size,
                    prefetch_distance,
                    callback);
            }
        }
        {
            SCOPED_BENCHMARK("Struct Array Chunked");
            foreach_element__struct_array__chunked(
                elements.data(), size, callback);
        }
        {
            SCOPED_BENCHMARK("Struct Array");
            foreach_element__struct_array(elements.data(), size, callback);
        }
        {
            SCOPED_BENCHMARK("Struct Array Zero Compare");
            foreach_element__struct_array__backwards(
                elements.data(), size, callback);
        }
    }

    std::cout << "\n\n";
    benchmark.print();
    std::cout << "\n\n";

    int error_count = 0;
    int expected_value = iterations * benchmark.amount();
    for (Element &element : elements) {
        if (element.value != expected_value) {
            std::cout << "Error!\n";
            error_count++;
        }
    }
    std::cout << "Errors: " << error_count << "\n";
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
