//
// Created by a1091793 on 11/9/17.
//

#ifndef GEON_OPT_IMPLEMENTATIONCOST_HPP
#define GEON_OPT_IMPLEMENTATIONCOST_HPP

#include <boost/config.hpp>
#include "ModuleAPI.hpp"
#include <iostream>

class ImplementationCost : public evalModuleAPI
{
public:
    ImplementationCost(){}

    std::string name() const
    {
        return ("Implementation Cost");
    }

    void configure(std::string _configure_string, boost::filesystem::path _geoproj_dir)
    {
        std::cout << "Configuring" << std::endl;
    }

    virtual double calculate(const std::vector<double>  & _real_decision_vars, const std::vector<int> & _int_decision_vars) const
    {
        return 1 / _real_decision_vars[0];
    }

    virtual MinOrMaxType isMinOrMax() const
    {
        return MINIMISATION;
    }

    ~ImplementationCost()
    {

    }

};


#endif //GEON_OPT_IMPLEMENTATIONCOST_HPP
