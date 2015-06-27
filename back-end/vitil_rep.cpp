/*****************************************************
 *  By Alan Zhuang (cheedoong@acm.org), APD, Tencent.
 *******************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map> //如果数据已经排序的话，考虑性能用这个更好
#endif

#include <set>
#include <libgen.h>
#include <errno.h>
#include "vitil_rep.h"
#include "cmysql.h"
#include "cexception.h"

using namespace std;
using namespace __gnu_cxx;

map<int, string> vip_cities;                            // vip cities' ids
map<int, string> vip_provinces;
map<int, string> provinces;
map<int, string> vip_isps;
map<int, string> prov_isps;
map<int, string>::iterator it_geo;              //检索、遍历神马的有用
map<int, string>::iterator it_isp;              //检索、遍历神马的有用
map<geo_isp_key, threshold> vip_cities_thre;    // 广深
map<geo_isp_key, threshold> vip_provinces_thre;
map<geo_isp_key, threshold> provinces_thre;     //
map<geo_isp_key, threshold>::iterator it_m;     //遍历用

struct tm p, hot_p, publish_p;

void get_conf(const char *conf_filename)
{
	ifstream inf;
	string si, s[8];
	int coln;

	cout << "get_conf" << endl;
	inf.open(conf_filename);
	while (getline(inf, si)) {
		istringstream iss(si);
		coln = 0;
		cout << si << endl;
		while (iss >> s[coln++]) ;
		cout << coln << " " << s[0].at(0) << endl;
		if (s[0].at(0) == '#')
			cout << "is #" << endl;
		else
			cout << "not #" << endl;
		//const char *c = s[0].c_str();
		int i = 0;
		if ((coln >= 6) && (s[0].at(0) != '#')) {
			i++;
			cout << i << s[2] << " " << s[3] << " " << s[5] << " " << s[6] << endl;
			geo_isp_key gi_key = { atoi(s[1].c_str()), atoi(s[4].c_str()) };
			threshold thre = { strtod(s[5].c_str(), NULL), strtod(s[6].c_str(), NULL) };
			if (s[0] == "PROV") {
				provinces.insert(pair<int, string>(atoi(s[1].c_str()), s[2]));
				provinces_thre.insert(pair<geo_isp_key, threshold>(gi_key, thre));
				prov_isps.insert(pair<int, string>(atoi(s[4].c_str()), s[3]));
			} else if (s[0] == "VIP_P") {
				vip_provinces.insert(pair<int, string>(atoi(s[1].c_str()), s[2]));
				vip_provinces_thre.insert(pair<geo_isp_key, threshold>(gi_key, thre));
				vip_isps.insert(pair<int, string>(atoi(s[4].c_str()), s[3]));
			} else if (s[0] == "VIP_C") {
				vip_cities.insert(pair<int, string>(atoi(s[1].c_str()), s[2]));
				vip_cities_thre.insert(pair<geo_isp_key, threshold>(gi_key, thre));
				vip_isps.insert(pair<int, string>(atoi(s[4].c_str()), s[3]));
			}
		}
	}
	inf.close();
}

int get_rates_over_count(char *geo_id_name, int geo_id, int isp_id, struct tm *tm_now,
			 double frate_thre, double brate_thre, double *frate, double *brate, int *visits, int *plays,
			 string &frate_str, string &brate_str)
{
	char datetime_now[20];
	char failure_table_name[20], second_buffer_table_name[20];
	char sql_failure[512], sql_second_buffer[512];

	sprintf(failure_table_name, "dD%04d%02d%02dT_5004", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
	sprintf(second_buffer_table_name, "dD%04d%02d%02dT_5005", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
	sprintf(datetime_now, "%04d-%02d-%02d %02d:%02d:%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
		tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

	double f_rate, b_rate;
	CMysql mysql_handle("10.166.149.48", "itil", "itil", "Target5Min");
	snprintf(sql_failure, sizeof(sql_failure), "select SUM(val * visitTimes)*100 / SUM(visitTimes) AS val, SUM(visitTimes) AS visitTimes, logTime from %s where logTime=\"%s\" and vt=203 and %s=%d and ispId=%d and platform=1 group by logTime;",
		 failure_table_name, datetime_now, geo_id_name, geo_id, isp_id);
	try
	{
		mysql_handle.Query(sql_failure);
		mysql_handle.StoreResult();
		stringstream ss;
		ss.flags(ios::fixed | ios::showpoint | ios::right);
		ss.precision(2);
		for (char **row = mysql_handle.FetchRow(); row; row = mysql_handle.FetchRow()) {
			if (row[0]) {
				if (row[1])
					(*visits) += atoi(row[1]);
				f_rate = strtod(row[0], NULL);
				ss << " " << f_rate;
				cout << "fail_rate: this: " << geo_id << " " << isp_id << "  " << f_rate << " thres: " << frate_thre << endl;
				(*frate) = f_rate;
			}
		}
		frate_str += ss.str();
		mysql_handle.FreeResult();
	}
	catch (CCommonException &e)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	snprintf(sql_second_buffer, sizeof(sql_failure), "select SUM(bufferCount)*100 / SUM(playCount) AS val, SUM(playCount) AS visitTimes, logTime from %s where logTime=\"%s\" and vt=203 and %s=%d and ispId=%d and platform=1 group by logTime;",
		 second_buffer_table_name, datetime_now, geo_id_name, geo_id, isp_id);
	try
	{
		mysql_handle.Query(sql_second_buffer);
		mysql_handle.StoreResult();
		stringstream ss;
		ss.flags(ios::fixed | ios::showpoint | ios::right);
		ss.precision(2);
		for (char **row = mysql_handle.FetchRow(); row; row = mysql_handle.FetchRow()) {
			if (row[0]) {
				if (row[1])
					(*plays) += atoi(row[1]);
				b_rate = strtod(row[0], NULL);
				ss << " " << b_rate;
				cout << "buffer_rate: this: " << geo_id << " " << isp_id << "  " << b_rate << " thres: " << brate_thre << endl;
				(*brate) = b_rate;
			}
		}
		brate_str += ss.str();
		mysql_handle.FreeResult();
	}
	catch (CCommonException &e)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int do_rep_vitil(struct tm tm_now[], int n)
{
	char geo_id_name[8];

	sprintf(geo_id_name, "%s", "cityId");
	for (it_m = vip_cities_thre.begin(); it_m != vip_cities_thre.end(); ++it_m) {
		int city_id = it_m->first.geo_id;
		string city_name = vip_cities[city_id];
		//cout << "222 " << city_id << " " << city_name << endl;
		int isp_id = it_m->first.isp_id;
		string isp_name = vip_isps[isp_id];
		//geo_isp_key gi_key = { city_id, isp_id };
		double frate_thre = it_m->second.frate;
		double brate_thre = it_m->second.brate;
		//cout << "fail_rate: " << city_name << " " << isp_name << " " << frate_thre << endl;
		int fcount = 0, bcount = 0, visits = 0, plays = 0;
		double f_rate[6] = { 0.0 }, b_rate[6] = { 0.0 };
		string frate_str = "", brate_str = "";
		for (int i = 0; i < n; i++)
			get_rates_over_count(geo_id_name, city_id, isp_id, &tm_now[i],
					     frate_thre, brate_thre, &f_rate[i], &b_rate[i], &visits, &plays,
					     frate_str, brate_str);
		char rep_content[128];
		count_over(f_rate, b_rate, frate_thre, brate_thre, &fcount, &bcount, n);
		if (visits > 60 && plays > 60) {
			if (fcount >= 6) {
				sprintf(rep_content, "%s %s 失败率严重 %s", city_name.c_str(), isp_name.c_str(), frate_str.c_str());
				do_report(5388297, rep_content);
				cout << "失败率严重: " << city_name << " " << isp_name << endl;
			} else if (fcount >= 4) {
				sprintf(rep_content, "%s %s 失败率一般 %s", city_name.c_str(), isp_name.c_str(), frate_str.c_str());
				do_report(5388296, rep_content);
				cout << "失败率一般: " << city_name << " " << isp_name << endl;
			}
			if (bcount >= 6) {
				sprintf(rep_content, "%s %s 二次缓冲率严重 %s", city_name.c_str(), isp_name.c_str(), brate_str.c_str());
				do_report(5388299, rep_content);
				cout << "二次缓冲率严重: " << city_name << " " << isp_name << endl;
			} else if (bcount >= 4) {
				sprintf(rep_content, "%s %s 二次缓冲率一般 %s", city_name.c_str(), isp_name.c_str(), brate_str.c_str());
				do_report(5388298, rep_content);
				cout << "二次缓冲率一般: " << city_name << " " << isp_name << endl;
			}
		}
	}

	sprintf(geo_id_name, "%s", "provId");
	for (it_m = vip_provinces_thre.begin(); it_m != vip_provinces_thre.end(); ++it_m) {
		int prov_id = it_m->first.geo_id;
		string prov_name = vip_provinces[prov_id];
		//cout << "222 " << city_id << " " << city_name << endl;
		int isp_id = it_m->first.isp_id;
		string isp_name = vip_isps[isp_id];
		//geo_isp_key gi_key = { prov_id, isp_id };
		double frate_thre = it_m->second.frate;
		double brate_thre = it_m->second.brate;
		//cout << "fail_rate: " << prov_name << " " << isp_name << " " << frate_thre << endl;
		int fcount = 0, bcount = 0, visits = 0, plays = 0;
		double f_rate[6] = { 0.0 }, b_rate[6] = { 0.0 };
		string frate_str = "", brate_str = "";
		for (int i = 0; i < n; i++)
			get_rates_over_count(geo_id_name, prov_id, isp_id, &tm_now[i],
					     frate_thre, brate_thre, &f_rate[i], &b_rate[i], &visits, &plays,
					     frate_str, brate_str);
		char rep_content[128];
		if (visits > 60 && plays > 60) {
			count_over(f_rate, b_rate, frate_thre, brate_thre, &fcount, &bcount, n);
			if (fcount >= 6) {
				sprintf(rep_content, "%s %s 失败率严重 %s", prov_name.c_str(), isp_name.c_str(), frate_str.c_str());
				do_report(5388297, rep_content);
				cout << "失败率严重: " << prov_name << " " << isp_name << endl;
			} else if (fcount >= 4) {
				sprintf(rep_content, "%s %s 失败率一般 %s", prov_name.c_str(), isp_name.c_str(), frate_str.c_str());
				do_report(5388296, rep_content);
				cout << "失败率一般: " << prov_name << " " << isp_name << endl;
			}
			if (bcount >= 6) {
				sprintf(rep_content, "%s %s 二次缓冲率严重 %s", prov_name.c_str(), isp_name.c_str(), brate_str.c_str());
				do_report(5388299, rep_content);
				cout << "二次缓冲率严重: " << prov_name << " " << isp_name << endl;
			} else if (bcount >= 4) {
				sprintf(rep_content, "%s %s 二次缓冲率一般 %s", prov_name.c_str(), isp_name.c_str(), brate_str.c_str());
				do_report(5388298, rep_content);
				cout << "二次缓冲率一般: " << prov_name << " " << isp_name << endl;
			}
		}
	}

	for (it_m = provinces_thre.begin(); it_m != provinces_thre.end(); ++it_m) {
		int prov_id = it_m->first.geo_id;
		string prov_name = provinces[prov_id];
		//cout << "222 " << city_id << " " << city_name << endl;
		int isp_id = it_m->first.isp_id;
		string isp_name = prov_isps[isp_id];
		//geo_isp_key gi_key = { prov_id, isp_id };
		double frate_thre = it_m->second.frate;
		double brate_thre = it_m->second.brate;
		//cout << "fail_rate: " << prov_name << " " << isp_name << " " << frate_thre << endl;
		int fcount = 0, bcount = 0, visits = 0, plays = 0;
		double f_rate[6] = { 0.0 }, b_rate[6] = { 0.0 };
		string frate_str = "", brate_str = "";
		for (int i = 0; i < n; i++)
			get_rates_over_count(geo_id_name, prov_id, isp_id, &tm_now[i],
					     frate_thre, brate_thre, &f_rate[i], &b_rate[i], &visits, &plays,
					     frate_str, brate_str);
		char rep_content[128];
		count_over(f_rate, b_rate, frate_thre, brate_thre, &fcount, &bcount, n);
		if (visits > 60 && plays > 60) {
			if (fcount >= 6) {
				sprintf(rep_content, "%s %s 失败率严重 %s", prov_name.c_str(), isp_name.c_str(), frate_str.c_str());
				do_report(5388297, rep_content);
				cout << "失败率严重: " << prov_name << " " << isp_name << endl;
			} else if (fcount >= 4) {
				sprintf(rep_content, "%s %s 失败率一般 %s", prov_name.c_str(), isp_name.c_str(), frate_str.c_str());
				do_report(5388296, rep_content);
				cout << "失败率一般: " << prov_name << " " << isp_name << endl;
			}
			if (bcount >= 6) {
				sprintf(rep_content, "%s %s 二次缓冲率严重 %s", prov_name.c_str(), isp_name.c_str(), brate_str.c_str());
				do_report(5388299, rep_content);
				cout << "二次缓冲率严重: " << prov_name << " " << isp_name << endl;
			} else if (bcount >= 4) {
				sprintf(rep_content, "%s %s 二次缓冲率一般 %s", prov_name.c_str(), isp_name.c_str(), brate_str.c_str());
				do_report(5388298, rep_content);
				cout << "二次缓冲率一般: " << prov_name << " " << isp_name << endl;
			}
		}
	}

	return 1;
}

int main(int argc, char *argv[])
{
	time_t t_now, t_before_30min;

	time(&t_now);
	t_now -= 1800;
	t_before_30min = t_now - 1800;
	//struct tm tm_now; // and ... tm_before_30min;
	struct tm tm_list[6];

	time_t t;
	int i = 0;
	for (t = t_before_30min, i = 0; t < t_now && i < 6; t++) {
		if (t % 300 == 0) {
			localtime_r(&t, &tm_list[i]);
			printf("now: %s", asctime(&tm_list[i]));
			i++;
		}
	}

	get_conf("vitil_rep.conf");

	do_rep_vitil(tm_list, 6);
}
