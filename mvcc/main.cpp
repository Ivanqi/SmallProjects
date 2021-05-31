#include "mvcc.hpp"
#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <cassert>

using namespace std;
using namespace chrono;
using namespace mvcc11;

namespace
{
    // auto hr_now() -> decltype(high_resolution_clock::now())
    // {
    //     return high_resolution_clock::now();
    // }

    auto INIT = "init";
    // auto OVERWRITTEN = "overwritten";
    // auto UPDATED = "updated";
    // auto DISTURBED = "disturbed";
};

template <class Mutex>
auto make_lock(Mutex& mtx) -> std::unique_lock<Mutex>
{
    return std::unique_lock<Mutex>{mtx};
}

template <class Mutex, class Computation>
auto locked(Mutex& mtx, Computation comp) -> decltype(comp())
{
    auto lock = make_lock(mtx);
    return comp();
}

#define LOCKED(mtx)                                         \
    if (bool locked_done_eiN8Aegu = false)                  \
        {}                                                  \
    else                                                    \
        for (auto mtx ## _lock_eiN8Aegu = make_lock(mtx));  \
            !locked_done_eiN8Aegu;                          \
            locked_done_eiN8Aegu = true)                    \

#define LOCKED_LOCK(mtx) mtx ## _lock_eiN8Aegu

void test_case_1() 
{
    mvcc<string> x;
    auto snapshot = *x;
    assert(snapshot->version == 0);
    assert(snapshot->value.empty() == true);
}

void test_case_2()
{
    mvcc<string> x{INIT};
    auto snapshot = x.current();

    assert(snapshot == x.current());
    assert(snapshot == *x);

    assert(snapshot != nullptr);
    assert(snapshot->version == 0);
    assert(snapshot->value == INIT);
}

int main() {
    
    test_case_1();
    return 0;
}
        