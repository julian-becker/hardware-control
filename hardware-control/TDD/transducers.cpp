//
//  transducers.cpp
//  hardware-control
//
//  Created by Julian Becker on 19.01.16.
//  Copyright (c) 2016 Julian Becker. All rights reserved.
//

#include "transducers.h"
#include <catch.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <streambuf>
#include <sstream>
#include <map>


struct state {
    std::vector<int> stack;
    std::map<std::string,std::function<void(state&,std::istream&)>> dict;
};

template <typename T>
void tryParse(state& s, std::istream& istr, std::function<T(std::istream&)> parse, std::function<void(state&,T)> action);

template <>
void tryParse(state& s, std::istream& istr, std::function<int(std::istream&)> parse, std::function<void(state&,int)> action) {
    auto pos = istr.tellg();
    try {
        auto val = parse(istr);
        action(s,val);
    }
    catch(...) {
        istr.clear();
        istr.seekg(pos);
    }
}

std::vector<int> forth(state& s, std::istream& istr) {
    auto pos = istr.tellg();
    std::string token;
    if(istr >> token) {
        std::istringstream str(token);
        int num;
        if(str >> num) {
            s.stack.push_back(num);
            return forth(s,istr);
        }
        
        if(s.dict.count(token)) {
            s.dict.at(token)(s,istr);
            return forth(s,istr);
        }
    }
    
    return s.stack;
}

TEST_CASE("transducers") {
    state s{};
    s.dict["+"] = [](state& s, std::istream& istr) {
        auto a = s.stack.back(); s.stack.pop_back();
        auto b = s.stack.back(); s.stack.pop_back();
        s.stack.push_back(a+b);
    };

    s.dict["-"] = [](state& s, std::istream& istr) {
        auto a = s.stack.back(); s.stack.pop_back();
        auto b = s.stack.back(); s.stack.pop_back();
        s.stack.push_back(b-a);
    };
    
    s.dict[":"] = [](state& s, std::istream& istr) {
        std::string wordname;
        auto pos = istr.tellg();
        try {
            if(istr >> wordname) {
                struct mybuf : public std::streambuf {};
                char buf[100u];
                istr.get(buf,sizeof(buf),';');
                char _;
                istr.get(_);
                std::string def(buf);
                s.dict[wordname] = [def](state& s, std::istream& istr1) {
                    std::istringstream defstr(def);
                    forth(s,defstr);
                };
            }
        }
        catch(...) {
            istr.seekg(pos);
            throw;
        }
    };
    
    
    SECTION("0") {
        std::istringstream stream("0");
        CHECK(forth(s, stream) == std::vector<int>{0});
    }
    
    SECTION("1") {
        std::istringstream stream("1");
        CHECK(forth(s, stream) == std::vector<int>{1});
    }
    
    SECTION("1 2") {
        std::istringstream stream("1 2");
        CHECK(forth(s, stream) == std::vector<int>({1, 2}));
    }
    
    SECTION("1 2 +") {
        std::istringstream stream("1 2 +");
        CHECK(forth(s, stream) == std::vector<int>({3}));
    }
    
    SECTION("10 2 - 100 +") {
        std::istringstream stream("10 2 - 100 +");
        CHECK(forth(s, stream) == std::vector<int>({108}));
    }
    
    SECTION(": plus6 1 + 5 + ; 3 plus6") {
        std::istringstream stream(": plus6 1 + 5 + ; 3 plus6");
        CHECK(forth(s, stream) == std::vector<int>({9}));
    }
    
    
    auto enumerate = [](auto step) {
        return [step,n=0](auto s, auto...ins) mutable {
            return step(s, n++, ins...);
        };
    };
    
}