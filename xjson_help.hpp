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

static json json_from_file(c_t path)
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