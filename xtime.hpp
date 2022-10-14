#pragma once
#include "xcom.hpp"
#include "xstr.hpp"

#define X_NS_IN_SEC (1000 * 1000 * 1000)
static void sleep_ns(uint64_t ns)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}

static void sleep_ms(uint64_t ms)
{
    sleep_ns(ms * 1000 * 1000);
}

static uint64_t get_ns()
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
//////////////////////////////////////////////////////////////////////////////////////



#define XX_TIME_FORMAT "%02d-%02d-%02d"
#define XX_DATE_FORMAT "%04d-%02d-%02d"
#define XX_DATE_TIME_FORMAT "%04d-%02d-%02d %02d-%02d-%02d"
#define XX_DATE_TIME_FORMAT2 "%04d-%02d-%02d_%02d_%02d_%02d"
class xx_time
{
public:
    xx_time()
    {
        clean();
    }
    xx_time(const string& str,c_t format = XX_DATE_TIME_FORMAT)
    {
        init(str.c_str(),format);
    }
    xx_time(time_t t)
    {
        init(t);
    }
    xx_time(time_t t,int diff)
    {
        init(t);
        day_diff(diff);
    }

    void day_diff(int n)
    {
        time_t t = to_time();
        t += n*24*3600;
        init(t);
    }

    // "yyyy-mm-dd hh:mm:ss"
    void init(c_t str,c_t format = XX_DATE_TIME_FORMAT)
    {
        clean();
        sscanf(str, format,
               &x_.tm_year, &x_.tm_mon, &x_.tm_mday,
               &x_.tm_hour, &x_.tm_min, &x_.tm_sec);
        to_sys_tm();
    }

    void init(time_t t)
    {
        clean();
        if(0 == t)
            t = time(nullptr);

        tm *tt = localtime(&t);
        x_ = *tt;
    }

    // yyyy-mm-dd
    string to_date_str(const char* format = XX_DATE_FORMAT)
    {
        to_user_tm();
        char temp[100] = {0};
        sprintf(temp, format,
                x_.tm_year,x_.tm_mon,x_.tm_mday);
        to_sys_tm();
        return temp;
    }
    // hh:mm:ss
    string to_time_str(const char* format = XX_TIME_FORMAT)
    {
        to_user_tm();
        char temp[100] = {0};
        sprintf(temp, format,
                x_.tm_hour,x_.tm_min,x_.tm_sec);
        to_sys_tm();
        return temp;
    }

    // yyyy-mm-dd hh:mm:ss
    string to_date_time_str(const char* format = XX_DATE_TIME_FORMAT)
    {
        to_user_tm();
        char temp[100] = {0};
        sprintf(temp, format,
                x_.tm_year,x_.tm_mon,x_.tm_mday,
                x_.tm_hour,x_.tm_min,x_.tm_sec);
        to_sys_tm();
        return temp;
    }

    time_t to_time(){
        return mktime(&x_);
    }
    
private:
    void clean()
    {
        memset(&x_, 0, sizeof(x_));
    }
    void to_sys_tm()
    {
        x_.tm_year -= 1900;
        x_.tm_mon -= 1;
    }

    void to_user_tm()
    {
        x_.tm_year += 1900;
        x_.tm_mon += 1;
    }
private:
    tm x_;
};


// yyyy-mm-dd
// []
static vector<string> get_time_in(c_t from, c_t to)
{
    xx_time xx_from;
    xx_time xx_to;

    xx_from.init(from);
    xx_to.init(to);

    vector<string> ret;
    while(true)
    {
        if(xx_from.to_time() > xx_to.to_time())
            break;
        ret.push_back(xx_from.to_date_str());
        xx_from.day_diff(1);
    }

    return ret;
}


static void time_test()
{
    const char* xxxx= "2020-05-08";
    xx_time xx;
    xx.init(xxxx);
    cout<<xx.to_date_str();
    cout<<endl;
    cout<<xx.to_date_time_str();
    cout<<endl;
    cout<<xx.to_time_str();
    cout<<endl;

    xx.day_diff(1);
    cout<<xx.to_date_time_str();
    cout<<endl;

    xx.day_diff(30);
    cout<<xx.to_date_time_str();
    cout<<endl;
}