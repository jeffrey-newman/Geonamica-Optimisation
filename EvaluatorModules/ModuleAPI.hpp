//
// Created by a1091793 on 11/9/17.
//

#ifndef GEON_OPT_MODULEAPI_HPP
#define GEON_OPT_MODULEAPI_HPP

#include "MinOrMax.hpp"
#include <string>
#include <vector>

class evalModuleAPI
{
public:
    virtual std::string name() const = 0;
    virtual void configure(std::string _configure_string) = 0;
    virtual double calculate(const std::vector<double>  & _real_decision_vars, const std::vector<int> & _int_decision_vars) const = 0;
    virtual MinOrMaxType isMinOrMax() const = 0;
    virtual ~evalModuleAPI(){}
};

#endif //GEON_OPT_MODULEAPI_HPP
