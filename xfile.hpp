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
	CHECK_RETV(NULL != p, -1);

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
		printf("xfile_read err,file=%s,err=%d\n", path.c_str(), errno);
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

static int xfile_xfile_write_lines(string path, vector<string> lines)
{
	string all = xmerge(lines, "\n");
	return xfile_write_new(path, all.c_str(), all.length());
}

static bool xfile_create_foder(string path)
{
	bool ret = std::filesystem::create_directories(path);
	return ret;
}

static bool xfile_exists(string path)
{
	bool ret = std::filesystem::exists(path);
	return ret;
}

static vector<string> xfile_files_in_dir(string path, bool recursive = false)
{
	vector<string> ret;
	if (recursive)
	{
		for (auto const &dir_entry : fs::recursive_directory_iterator(path))
		{
			if(dir_entry.is_regular_file())
				ret.push_back(dir_entry.path());
		}
	}
	else
	{
		for (auto const &dir_entry : fs::directory_iterator(path))
		{
			if(dir_entry.is_regular_file())
				ret.push_back(dir_entry.path());
		}
	}

	return ret;
}

static vector<string> xfile_dirss_in_dir(string path, bool recursive = false)
{
	vector<string> ret;
	if (recursive)
	{
		for (auto const &dir_entry : fs::recursive_directory_iterator(path))
		{
			if(dir_entry.is_directory())
				ret.push_back(dir_entry.path());
		}
	}
	else
	{
		for (auto const &dir_entry : fs::directory_iterator(path))
		{
			if(dir_entry.is_directory())
				ret.push_back(dir_entry.path());
		}
	}

	return ret;
}
