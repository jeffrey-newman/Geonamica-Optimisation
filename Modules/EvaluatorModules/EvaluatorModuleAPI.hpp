//
// Created by a1091793 on 11/9/17.
//

#ifndef GEON_OPT_MODULEAPI_HPP
#define GEON_OPT_MODULEAPI_HPP

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "MinOrMax.hpp"

class EvalModuleAPI
{
public:
    virtual const std::string name() const = 0;
    virtual void configure(const std::string _configure_string, const boost::filesystem::path _geoproj_dir) = 0;
    virtual std::shared_ptr<const std::vector<double> > calculate(const std::vector<double>  & _real_decision_vars, const std::vector<int> & _int_decision_vars) = 0;
    virtual const std::vector<MinOrMaxType>& isMinOrMax() const = 0;
    virtual ~EvalModuleAPI(){}
};

#endif //GEON_OPT_MODULEAPI_HPP
