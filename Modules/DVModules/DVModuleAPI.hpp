//
// Created by a1091793 on 19/01/18.
//

#ifndef GEON_OPT_DVMODULEAPI_HPP
#define GEON_OPT_DVMODULEAPI_HPP

#include <string>
#include <boost/filesystem.hpp>
#include <vector>

class DVModuleAPI
{
public:

    template <typename T>
    struct Bounds
    {
        std::vector<T> upper_bounds;
        std::vector<T> lower_bounds;
    };

    virtual const std::string name() const = 0;
    virtual void configure(const std::string _configure_string, const boost::filesystem::path _geoproj_dir) = 0;
    virtual void setDVs(const std::vector<double>  & _real_decision_vars, const std::vector<int> & _int_decision_vars, const boost::filesystem::path _geoproj_file) const = 0;
    virtual const Bounds<double>& realBounds() const = 0;
    virtual const Bounds<int>& intBounds() const = 0;
};

#endif //GEON_OPT_DVMODULEAPI_HPP
