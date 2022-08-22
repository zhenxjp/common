#pragma once
#include "json.hpp"
#include "com.hpp"
#include "file.hpp"

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

static string json_to_mem(json j)
{
    return to_string(j);
}

static bool json_to_file(json j,c_t path)
{
    string mem = json_to_mem(j);
    xfile_write_new(path,mem.c_str(),mem.length());
    return true;
}