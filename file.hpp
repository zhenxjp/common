#pragma once
#include "com.hpp"
#include "str.hpp"

static void xfile_del(const char *path)
{
	remove(path);
}

static long xfile_get_size(const char *pPath)
{
	FILE *p = fopen(pPath, "r");
	CHECK_RETV(NULL != p, -1);

	fseek(p, 0, SEEK_END);
	long lRet = ftell(p);
	fclose(p);
	return lRet;
}

static string xfile_read(const char *pPath)
{
	long len = xfile_get_size(pPath);
	if (len < 0)
	{
		printf("xfile_read err,file=%s,err=%d\n", pPath, errno);
		return "";
	}

	string ss;
	ss.resize(len);

	FILE *p = fopen(pPath, "rb");
	CHECK_RETV(NULL != p, "");

	size_t ret = fread(&ss[0], 1, len, p);
	fclose(p);
	return ss;
}

static int xfile_write_append(const char *pFile, const char *p, size_t nLen, int32_t nOff = 0)
{
	FILE *PF;
	PF = fopen(pFile, "ab+");
	if (nullptr == PF)
	{
		printf("xfile_read err,file=%s,err=%d\n", pFile, errno);
		return 0;
	}
	CHECK_RETV(0 == fseek(PF, nOff, SEEK_SET), -1);
	size_t nRet = fwrite(p, 1, nLen, PF);
	CHECK_RETV(nRet == nLen, -2);
	fflush(PF);
	fclose(PF);
	return 0;
}

static int xfile_write_new(const char *pFile, const char *p, size_t nLen)
{
	xfile_del(pFile);
	return xfile_write_append(pFile, p, nLen, 0);
}

static vector<string> xfile_read_lines(const char *pPath)
{
	string all = xfile_read(pPath);
	vector<string> lines = xsplit(all, "\n");
	return lines;
}

static int xfile_xfile_write_lines(const char *pFile, vector<string> lines)
{
	string all = xmerge(lines, "\n");
	return xfile_write_new(pFile, all.c_str(), all.length());
}

static bool xfile_create_foder(c_t path)
{
	bool ret = std::filesystem::create_directories(path);
	return ret;
}

static bool xfile_exists(c_t path)
{
	bool ret = std::filesystem::exists(path);
	return ret;
}

static vector<string> xfile_files_in_dir(c_t path, bool recursive = false)
{
	vector<string> ret;
	if (recursive)
	{
		for (auto const &dir_entry : fs::recursive_directory_iterator(path))
		{
			string path = dir_entry.path();
			//path = path.substr(1,path.length() - 2);
			ret.push_back(dir_entry.path());
		}
	}
	else
	{
		for (auto const &dir_entry : fs::directory_iterator(path))
		{
			string path = dir_entry.path();
			//path = path.substr(1,path.length() - 2);
			ret.push_back(dir_entry.path());
		}
	}

	return ret;
}