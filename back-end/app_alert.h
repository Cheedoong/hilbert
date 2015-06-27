#ifndef _APP_ALERT_H_
#define _APP_ALERT_H_

#include <string.h>
#include <stdint.h>
#include <iconv.h>
#include <string>
using std::string;

class CodeConverter {
private:
iconv_t cd;
public:
// ����
CodeConverter(const char *from_charset, const char *to_charset)
{
	cd = iconv_open(to_charset, from_charset);
}
// ����
~CodeConverter()
{
	iconv_close(cd);
}
// ת�����
int convert(char *inbuf, int inlen, char *outbuf, int outlen)
{
	char **pin = &inbuf;
	char **pout = &outbuf;

	memset(outbuf, 0, outlen);
	return iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen);
}
};

#endif
