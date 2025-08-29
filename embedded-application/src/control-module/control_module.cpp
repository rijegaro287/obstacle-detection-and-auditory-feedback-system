#include <iostream>

#include "control_module.hpp"

#include <thread>
#include <mutex>

using namespace std;

mutex mtx;

int main() {
    lock_guard<mutex> guard(mtx);
    // Critical section

    
    return 0;
}
