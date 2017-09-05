#ifndef _UTIL_H_
#define _UTIL_H_

#include <rpc.h>
#include <string>

#define CLSID_LENGTH 40

class CUtil
{
public:
  static GUID NewGuid();
  static GUID StringToGuid(const char *guid);
  static std::string GuidToString(GUID guid);

private:
  CUtil() {};
};

#endif