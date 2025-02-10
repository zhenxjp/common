#pragma once
#include "xcom.hpp"
#include "xstr.hpp"


static void xfile_del(string path)
{
	remove(path.c_str());
}

static long xfile_get_size(string path)
{
	FILE *p = fopen(path.c_str(), "r");
	if(nullptr == p){
		printf("get_size err,path = %s",path.c_str());
		return 0;
	}
	

	fseek(p, 0, SEEK_END);
	long lRet = ftell(p);
	fclose(p);
	return lRet;
}

static string xfile_read(string path)
{
	long len = xfile_get_size(path);
	if (len < 0)
	{
		printf("xfile_read err,file=%s,err=%d\n", path.c_str(), errno);
		return "";
	}

	string ss;
	ss.resize(len);

	FILE *p = fopen(path.c_str(), "rb");
	CHECK_RETV(NULL != p, "");

	size_t ret = fread(&ss[0], 1, len, p);
	fclose(p);
	return ss;
}

static int xfile_write_append(string path, const char *p, size_t nLen, int32_t nOff = 0)
{
	FILE *PF;
	PF = fopen(path.c_str(), "ab+");
	if (nullptr == PF)
	{
		printf("xfile_write_append err,file=%s,err=%d\n", path.c_str(), errno);
		return 0;
	}
	CHECK_RETV(0 == fseek(PF, nOff, SEEK_SET), -1);
	size_t nRet = fwrite(p, 1, nLen, PF);
	CHECK_RETV(nRet == nLen, -2);
	fflush(PF);
	fclose(PF);
	return 0;
}

static int xfile_write_new(string path, const char *p, size_t nLen)
{
	xfile_del(path);
	return xfile_write_append(path, p, nLen, 0);
}

static int xfile_write_new_str(string path, const string &str)
{
	return xfile_write_new(path, str.c_str(), str.size());
}

static vector<string> xfile_read_lines(string path)
{
	string all = xfile_read(path);
	vector<string> lines = xsplit(all, "\n");
	return lines;
}

static string xfile_fix_foder(string &foder)
{
	if(foder.empty())
	{
		foder = "./";
	}else
	{
		if(foder[foder.size() - 1] != '/')
		{
			foder += "/";
		}
	}
	return foder;
}
//////////////////////////////////////////////////////////////////////////


static int xfile_xfile_write_lines(string path, vector<string> lines)
{
	string all = xmerge(lines, "\n");
	return xfile_write_new(path, all.c_str(), all.length());
}

static bool xfile_exists(const string& path)
{
	return access(path.c_str(), F_OK) == 0;
}



// 递归创建多层目录的函数
bool xfile_create_foder(const std::string& path, mode_t mode = 0774) {
    size_t pos = 0;
    std::string tempPath;
    // 处理路径为空的情况
    if (path.empty()) {
        return false;
    }
    // 遍历路径中的每一级目录
    while ((pos = path.find('/', pos)) != std::string::npos) {
        // 截取当前路径的前缀
        tempPath = path.substr(0, pos);
        // 如果前缀不为空，则尝试创建该目录
        if (!tempPath.empty()) {
            if (mkdir(tempPath.c_str(), mode) != 0 && errno != EEXIST) {
                return false;
            }
        }
        ++pos;
    }
    // 尝试创建完整的路径
    if (mkdir(path.c_str(), mode) != 0 && errno != EEXIST) {
        return false;
    }
    return true;
}

// 检查路径是否是目录
bool is_directory(const string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return false;
    return S_ISDIR(statbuf.st_mode);
}

// 检查路径是否是普通文件
bool is_regular_file(const string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return false;
    return S_ISREG(statbuf.st_mode);
}

// 获取目录中的所有文件
vector<string> xfile_files_in_dir(const string& path, bool recursive = false) {
    vector<string> ret;
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return ret;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        string full_path = path + "/" + name;

        if (is_regular_file(full_path)) {
            ret.push_back(full_path);
        } else if (recursive && is_directory(full_path)) {
            auto subdir_files = xfile_files_in_dir(full_path, recursive);
            ret.insert(ret.end(), subdir_files.begin(), subdir_files.end());
        }
    }

    closedir(dir);
    return ret;
}

// 获取目录中的所有子目录
vector<string> xfile_dirs_in_dir(const string& path, bool recursive = false) {
    vector<string> ret;
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return ret;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        string full_path = path + "/" + name;

        if (is_directory(full_path)) {
            ret.push_back(full_path);
            if (recursive) {
                auto subdir_dirs = xfile_dirs_in_dir(full_path, recursive);
                ret.insert(ret.end(), subdir_dirs.begin(), subdir_dirs.end());
            }
        }
    }

    closedir(dir);
    return ret;
}

