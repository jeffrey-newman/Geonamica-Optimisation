//
// Created by a1091793 on 18/11/17.
//

#ifndef GEON_OPT_ZONALCOSTINGCELLCOUNT_H
#define GEON_OPT_ZONALCOSTINGCELLCOUNT_H


#include <vector>
#include <boost/filesystem.hpp>
#include "../../Pathify.hpp"
#include "EvaluatorModuleAPI.hpp"

struct ZoneCostingParameters
{
    CmdLinePaths project_path;
    CmdLinePaths zonal_raster;
    std::vector<int> exclusion_vals;
};

class ZonalCostingCellCount : public EvalModuleAPI
{

public:
    ZonalCostingCellCount()
    {
        min_or_max_types.push_back(MINIMISATION);
    }

    const std::string
    name() const;

    void
    configure(const std::string _configure_string, const boost::filesystem::path _geoproj_dir);

    virtual std::shared_ptr<const std::vector<double> >
    calculate(const std::vector<double> &_real_decision_vars, const std::vector<int> &_int_decision_vars);

    virtual const std::vector<MinOrMaxType>&
    isMinOrMax() const;

    ~ZonalCostingCellCount()
    {

    }

private:
    ZoneCostingParameters params;
    std::vector<MinOrMaxType> min_or_max_types;

};


#endif //GEON_OPT_ZONALCOSTINGCELLCOUNT_H
