#include <string>

namespace Snowball {

class Stemmer {
  public:
    virtual std::string operator()(const std::string& word) = 0;
};

Stemmer* make_stemmer(const char* language);

}
