//
// Created by a1091793 on 18/11/17.
//

#ifndef GEON_OPT_ZONALCOSTINGCELLCOUNT_H
#define GEON_OPT_ZONALCOSTINGCELLCOUNT_H


#include <vector>
#include <boost/filesystem.hpp>
#include "../Pathify.hpp"
#include "ModuleAPI.hpp"

struct ZoneCostingParameters
{
    CmdLinePaths project_path;
    CmdLinePaths zonal_raster;
    std::vector<int> exclusion_vals;
};

class ZonalCostingCellCount : public evalModuleAPI
{

public:
    ZonalCostingCellCount()
    {}

    std::string
    name() const;

    void
    configure(std::string _configure_string, boost::filesystem::path _geoproj_dir);

    virtual double
    calculate(const std::vector<double> &_real_decision_vars, const std::vector<int> &_int_decision_vars) const;

    virtual MinOrMaxType
    isMinOrMax() const;

    ~ZonalCostingCellCount()
    {

    }

private:
    ZoneCostingParameters params;

};


#endif //GEON_OPT_ZONALCOSTINGCELLCOUNT_H
