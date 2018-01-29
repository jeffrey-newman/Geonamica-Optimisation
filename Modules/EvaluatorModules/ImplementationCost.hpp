//
// Created by a1091793 on 11/9/17.
//

#ifndef GEON_OPT_IMPLEMENTATIONCOST_HPP
#define GEON_OPT_IMPLEMENTATIONCOST_HPP

#include <boost/config.hpp>
#include "EvaluatorModuleAPI.hpp"
#include <iostream>

class ImplementationCost : public EvalModuleAPI
{
public:
    ImplementationCost()
    {
        min_or_max_types.push_back(MINIMISATION);
    }

    virtual const std::string name() const
    {
        return ("Implementation Cost");
    }

    virtual void configure(const std::string _configure_string, const boost::filesystem::path _geoproj_dir)
    {
        std::cout << "Configuring" << std::endl;
    }

    virtual std::shared_ptr<const std::vector<double> > calculate(const std::vector<double>  & _real_decision_vars, const std::vector<int> & _int_decision_vars)
    {
        std::shared_ptr<std::vector<double> > return_vec(new std::vector<double>);
        return_vec->push_back(1 / _real_decision_vars[0]);
        return (return_vec);
    }

    virtual const std::vector<MinOrMaxType> & isMinOrMax() const
    {
        return (min_or_max_types);
    }

    ~ImplementationCost()
    {

    }

private:
    std::vector<MinOrMaxType> min_or_max_types;

};


#endif //GEON_OPT_IMPLEMENTATIONCOST_HPP
