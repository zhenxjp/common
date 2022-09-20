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

static string get_tm_date_str(tm *tt)
{
    stringstream ss;
    ss<<1900+tt->tm_year<<"-"<<1+tt->tm_mon<<"-"<<tt->tm_mday;
    return ss.str();
}

static string get_now_date_str()
{
    time_t now = time(nullptr);
    tm *tt= localtime(&now);
    return get_tm_date_str(tt);
}



static string get_tm_str(tm *tt)
{
    stringstream ss;
    ss<<1900+tt->tm_year<<"-"<<1+tt->tm_mon<<"-"<<tt->tm_mday
        <<" "
        <<tt->tm_hour<<":"<<tt->tm_min<<":"<<tt->tm_sec;
    return ss.str();
}


