#pragma once
#include "json.hpp"
#include "xcom.hpp"
#include "xfile.hpp"

using namespace std;
using namespace nlohmann;

static json json_from_mem(c_t mem)
{
    json j = json::parse(mem);

    return j;
}

static json json_from_file(string path)
{
    string mem = xfile_read(path);
    return json_from_mem(mem.c_str());
}

static string json_to_mem(json j, int indent = 1)
{
    return j.dump(indent);
}

static bool json_to_file(json j, c_t path)
{
    string mem = json_to_mem(j);
    xfile_write_new(path, mem.c_str(), mem.length());
    return true;
}

static json json_merge(vector<json> &all)
{
    json all_one;
    for (auto o : all)
    {
        all_one.push_back(o);
    }
    return all_one;
}

static map<string,string> json_get_map(json j)
{
    map<string,string> ret;
    auto jsons = j.get<vector<json>>();
    for(auto xx:jsons)
    {
        auto r_keys = xx.get<vector<string>>();
        string old_val = r_keys[0];
        string new_val = r_keys[1];
        ret[old_val] = new_val;
    }
    return ret;
}

template<class KEY_TYPE,class VAL_TYPE>
static map<KEY_TYPE,VAL_TYPE> json_get_map2(json j,string key,string val)
{
    map<KEY_TYPE,VAL_TYPE> ret;
    auto jsons = j.get<vector<json>>();
    for(auto xx:jsons)
    {
        KEY_TYPE k = xx[key].get<KEY_TYPE>();
        VAL_TYPE v = xx[val].get<VAL_TYPE>();
        ret[k] = v;
    }
    return ret;
}