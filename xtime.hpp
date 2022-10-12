#pragma once
#include "xcom.hpp"
#include "xstr.hpp"

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

// get "yyyy-mm-dd" by tm
static string get_tm_date_str(tm *tt)
{
    char temp[100]  = {0};
    sprintf(temp,"%d-%02d-%02d",
        1900+tt->tm_year,
        1+tt->tm_mon,
        tt->tm_mday);

    return temp;
}

// get "yyyy-mm-dd" by now
static string get_now_date_str(int diff_day = 0)
{
    time_t now = time(nullptr);
    now -= diff_day*24*3600;
    tm *tt= localtime(&now);
    return get_tm_date_str(tt);
}


// get "yyyy-mm-dd hh:mm:ss" bt tm
static string get_tm_str(tm *tt)
{
    stringstream ss;
    ss<<1900+tt->tm_year<<"-"<<1+tt->tm_mon<<"-"<<tt->tm_mday
        <<" "
        <<tt->tm_hour<<":"<<tt->tm_min<<":"<<tt->tm_sec;
    return ss.str();
}

// get "yyyy-mm-dd__hh_mm_ss" by tm
static string get_tm_str2(tm *tt)
{
    stringstream ss;
    ss<<1900+tt->tm_year<<"-"<<1+tt->tm_mon<<"-"<<tt->tm_mday
        <<"__"
        <<tt->tm_hour<<"_"<<tt->tm_min<<"_"<<tt->tm_sec;
    return ss.str();
}

// get "yyyy-mm-dd__hh_mm_ss" by now
static string get_now_str2()
{
    time_t now = time(nullptr);
    tm *tt= localtime(&now);
    return get_tm_str2(tt);
}
// yyyy-mm-dd
static tm str2tm(string date)
{
    tm tt;
    memset(&tt,0,sizeof(tt));
    vector<string> temp = xsplit(date,"-");
    tt.tm_year = stoll(temp[0]) - 1900;
    tt.tm_mon = stoll(temp[1]) - 1;
    tt.tm_mday = stoll(temp[2]);
    return tt;
}

// yyyy-mm-dd
static time_t str2time_t(string date)
{
    tm tt =  str2tm(date);
    return mktime(&tt);
}

// yyyy-mm-dd
static vector<string> get_time_in(c_t from,c_t to)
{
    vector<string> ret;
    time_t from_t = str2time_t(from) + 3600 * 1;
    time_t to_t = str2time_t(to) + 3600 * 10;

    for(time_t it = from_t;it < to_t; it+= 24*3600)
    {
        tm *tt= localtime(&it);
        string date_str =get_tm_date_str(tt);
        ret.push_back(date_str);
    }

    return ret;
}
