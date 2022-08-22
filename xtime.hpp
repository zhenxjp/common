#pragma once
#include "xcom.hpp"

#define X_NS_IN_SEC (1000*1000*1000)
static void sleep_ns(uint64_t ns){
    std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}

static void sleep_ms(uint64_t ms){
    sleep_ns(ms*1000*1000);
}

static uint64_t get_ns()
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}