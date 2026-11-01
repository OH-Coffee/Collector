
#ifndef matcher_hpp
#define matcher_hpp

#include <memory>
#include <string>

struct MatcherArgs
{
    std::string delim;
    std::string pattern;
};

class Matcher;
typedef std::shared_ptr<Matcher> MatcherPtr;

class Matcher
{
public:
             Matcher() = default;
    virtual ~Matcher() = default;
    
public:
    virtual bool match(const char        *i_object) const { return false;                         }
    virtual bool match(const std::string &i_object) const { return this->match(i_object.c_str()); }
public:
    virtual void clear()                              = 0;
    virtual bool insert(const std::string &i_pattern) = 0;
public:
    template<typename T> static void insert(MatcherPtr i_matcher, const T &i_container){
        for(const auto &it : i_container) { i_matcher->insert(it); }
    }
};

#endif /* matcher_hpp */

