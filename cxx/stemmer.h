#include <string>

namespace Snowball {

class Stemmer {
  public:
#ifdef SNOWBALL_WIDE
    virtual std::wstring operator()(const std::wstring& word) = 0;
#else
    virtual std::string operator()(const std::string& word) = 0;
#endif
};

Stemmer* make_stemmer(const char* language);

}
