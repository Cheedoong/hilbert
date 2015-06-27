#ifndef TO_RES_H_INCLUDED
#define TO_RES_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include "app_alert.h"

using namespace std;

struct geo_isp_key {
	int	geo_id;       //ciry_id or province_id
	int	isp_id;

	bool operator <(const geo_isp_key &gi_key) const
	{
		return (geo_id < gi_key.geo_id) || (geo_id == gi_key.geo_id && isp_id < gi_key.isp_id);
	}

	bool operator ==(const geo_isp_key &gi_key) const
	{
		return (geo_id == gi_key.geo_id) && (isp_id == gi_key.isp_id);
	}
};

struct threshold {
	double	frate;          // 失败率
	double	brate;          // 二次缓冲率
};

string get_basename(const string& full_path) //从完整路径里提取文件名
{
	char *ibase, *tmp;

	tmp = new char [full_path.size() + 1];
	//cout<<full_path.size()<<"  "<<full_path.length()<<endl;
	strcpy(tmp, full_path.c_str());
	ibase = basename(tmp);
	string ret(ibase);
	delete[] tmp; //删除new出来的东东，防止内存泄漏
	return ret;
}

string get_ori_filename(const string& filename) //提取原始文件名
{
	if (filename[0] == '.')
		return filename;
	size_t length = filename.size();
	if (filename.rfind(".mp4") != length - 4 && filename.rfind(".flv") != length - 4 && filename.rfind(".mlv") != length - 4)
		return filename;
	size_t count_dot = 0, i;
	for (i = length - 1; i; i--)
		if (filename[i] == '.')
			count_dot++;
	if (count_dot < 3)
		return filename;
	if (count_dot == 3) {
		size_t idx1, idx2;
		char *tmp = new char[length + 1];
		strcpy(tmp, filename.c_str());
		for (i = length; i && tmp[i] != '.'; i--) ;
		idx2 = i;
		for (i--; i && tmp[i] != '.'; i--) ;
		idx1 = i;
		for (i = idx1 + 1; tmp[i + idx2 - idx1]; i++) tmp[i] = tmp[i + idx2 - idx1];
		tmp[i] = '\0';
		string ret(tmp);
		delete[] tmp;
		return ret;
	} else {
		cerr << "Warning: Meet Abnormal Filename Once! " << filename << endl;
		return filename;
	}
}

string get_ori_filename_lite(const string& filename) //提取原始文件名
{
	size_t length = filename.size();
	size_t count_dot = 0, i;

	for (i = length - 1; i; i--)
		if (filename[i] == '.')
			count_dot++;
	if (count_dot < 3)
		return filename;
	if (count_dot == 3) {
		size_t idx1, idx2;
		char *tmp = new char[length + 1];
		strcpy(tmp, filename.c_str());
		for (i = length; i && tmp[i] != '.'; i--) ;
		idx2 = i;
		for (i--; i && tmp[i] != '.'; i--) ;
		idx1 = i;
		for (i = idx1 + 1; tmp[i + idx2 - idx1]; i++) tmp[i] = tmp[i + idx2 - idx1];
		tmp[i] = '\0';
		string ret(tmp);
		delete[] tmp;
		return ret;
	} else {
		cerr << "Warning: Meet Abnormal Filename Once! " << filename << endl;
		return filename;
	}
}

struct str_hash //对string的哈希函数
{
	size_t operator()(const string& str) const
	{
		unsigned long __h = 0;

		for (size_t i = 0; i < str.size(); i++)
			__h = 131 * __h + str[i];
		return size_t(__h);
	}
};


time_t get_time(const string& vtime) //从“2012-04-19”格式的时间中提取1971年到现在的秒数时间
{
	int yyyy, mm, dd;
	struct tm ttime;
	char *tmp = new char [vtime.size() + 1];

	strcpy(tmp, vtime.c_str());
	sscanf(tmp, "%d-%d-%d", &yyyy, &mm, &dd);
	delete[] tmp;
	ttime.tm_year = yyyy - 1900;
	ttime.tm_mon = mm - 1;
	ttime.tm_mday = dd;
	ttime.tm_hour = 0;
	ttime.tm_min = 0;
	ttime.tm_sec = 0;
	//cout<<yyyy<<"  "<<mm<<"  "<<dd<<endl;
	return mktime(&ttime);
}

void do_report(int rep_id, char *content)
{
	char rep_path[] = "/usr/local/apdtools/agent/reportexception";
	char cmd[1024];
	char utf8content[128];

	CodeConverter cc = CodeConverter("gb2312", "utf-8");

	cc.convert(content, strlen(content), utf8content, sizeof(utf8content));

	sprintf(cmd, "%s/reportexception  %s/reportException.conf %d \"%s\"",
		rep_path, rep_path, rep_id, utf8content);
	printf("CMD: %s\n", cmd);
	system(cmd);
}

void count_over(double *frate, double *brate, double frate_thre, double brate_thre, int *fcount, int *bcount, int n)
{
	int tmp_fcount = 0, tmp_bcount = 0;

	for (int i = 0; i < n; i++) {
		if (frate[i] > frate_thre) {
			tmp_fcount++;
		} else {
			if (tmp_fcount > *fcount)
				(*fcount) = tmp_fcount;
			tmp_fcount = 0;
		}
	}
	if (tmp_fcount > *fcount)
		(*fcount) = tmp_fcount;

	for (int i = 0; i < n; i++) {
		if (brate[i] > brate_thre) {
			tmp_bcount++;
		} else {
			if (tmp_bcount > *bcount)
				(*bcount) = tmp_bcount;
			tmp_bcount = 0;
		}
	}
	if (tmp_bcount > *bcount)
		(*bcount) = tmp_bcount;
}

#endif // FAST_DELETE_H_INCLUDED
