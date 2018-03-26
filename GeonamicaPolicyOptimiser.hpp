//
//  ZonalPolicyOptimiser.hpp
//  GeonamicaOptimiser
//
//  Created by a1091793 on 4/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef ZonalPolicyOptimiser_hpp
#define ZonalPolicyOptimiser_hpp


#include "Evaluation.hpp"
#include <stdio.h>
#include "GeonamicaPolicyParameters.hpp"
#include <boost/filesystem.hpp>
#include <blink/raster/gdal_raster.h>
#include <unordered_set>
#include <vector>
#include "OpenCVWriterClassified.hpp"
#include "OpenCVWriterGradient.hpp"
#include <pugixml.hpp>
#include "Modules/EvaluatorModules/EvaluatorModuleAPI.hpp"
#include "Modules/DVModules/DVModuleAPI.hpp"

struct MapObj
{
    enum ObjType{MINIMISATION, MAXIMISATION};
    ObjType type;
    CmdLinePaths file_path;
    std::vector<int> years;
    double discount_rate;
    int year_present_val;
};

struct XPathDV
{
    enum DVType{INTEGER_ORDERED, INTEGER_UNORDERED, REAL};
    struct Point
    {
        double x;
        double y;
    };
    DVType dv_type;
    std::string xpath_2_node;
    std::string attribute_name;
    std::vector<Point> base_spline;
    int i_lower_bounds;
    int i_upper_bounds;
    double d_lower_bounds;
    double d_upper_bounds;

};

struct SaveMapDetails
{
    enum SaveType{CATEGORISED, LINEAR_GRADIENT};
    SaveType type;
    CmdLinePaths legend_file;
    CmdLinePaths source_raster;
    std::vector<int> years;
    CmdLinePaths diff_raster;
    CmdLinePaths save_path;
    boost::shared_ptr<ColourMapperClassified> classified_clr_map;
    boost::shared_ptr<OpenCVWriterClassified> classified_writer;
    boost::shared_ptr<ColourMapperGradient> gradient_clr_map;
    boost::shared_ptr<OpenCVWriterGradient> gradient_writer;

};

class GeonamicaOptimiser : public ObjectivesAndConstraintsBase
{
private:
    
    GeonamicaPolicyParameters params;

    boost::filesystem::path original_bck_geoproj;
    boost::filesystem::path working_project;

    boost::filesystem::path zonal_map_path;

//    std::vector<boost::filesystem::path> obj_map_paths;
//    std::vector<std::pair<boost::filesystem::path, std::vector<int> > > obj_map_paths_minisation;
//    std::vector<boost::filesystem::path> obj_map_paths_maximisation;
    std::vector<MapObj> map_objectives;
    std::vector<boost::shared_ptr<EvalModuleAPI> > objective_modules;
    std::vector<boost::shared_ptr<DVModuleAPI> > dv_modules;
    std::vector<std::pair<int, int> > dv_modules_dv_int_subvector_loc;
    std::vector<std::pair<int, int> > dv_modules_dv_real_subvector_loc;

    boost::filesystem::path zones_delineation_map_path;
    blink::raster::gdal_raster<int> zones_delineation_map;
    boost::optional<int> zones_delineation_no_data_val;
    std::vector<int> zone_categories;
    std::set<int> delineations_ids;
    std::vector<int> zone_id_lookup; // maps the index of the vector as the nth zonal decision variable to the subregion id in subregion map.
    int min_delineated_id;
    int max_delineated_id;
    
    boost::filesystem::path logfile;
    boost::filesystem::path previous_logfile;

//    std::string temp_dir_template;
    
    
//    int analysisNum;
    int eval_count;
    
    int num_objectives = 0;
    int num_constraints = 0;

//    int min_zonal_dv_values  = 0; //lower bounds
//    int max_zonal_dv_values = 3; // upper bound of x
    // Decision variable mapping:
    // 0 -- 0 Strictly restrict
    // 1 -- 0.5 Weakly restrict
    // 2 -- 1 Allow
    // 3 -- 1.5 Stimulate
//    std::map<int, float> zone_policies = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
//    std::vector<int> zone_policies_vec = {1,2,3,4};

//    std::vector<std::string> int_dv_xpaths;
//    std::vector<std::string> real_dv_xpaths;
//    std::vector<std::string> spline_proportion_xpaths;
//    std::vector< std::vector< std::vector<double> > > spline_curves;

    std::vector<XPathDV> xpath_dvs;

    std::vector<int> int_lowerbounds;
    std::vector<int> int_upperbounds;
    std::vector<double > real_lowerbounds;
    std::vector<double > real_upperbounds;

    ProblemDefinitionsSPtr prob_defs;
    std::pair<std::vector<double>, std::vector<double> > objectives_and_constraints;

    std::vector<SaveMapDetails> save_img_rqsts;
//    std::vector<std::tuple<boost::filesystem::path, boost::filesystem::path, boost::shared_ptr<ColourMapperClassified>,     boost::shared_ptr<OpenCVWriterClassified>, std::string > > classified_img_rqsts;
//    std::vector<std::tuple<boost::filesystem::path, boost::filesystem::path, boost::shared_ptr<ColourMapperGradient>, boost::shared_ptr<OpenCVWriterGradient>, std::string > > lin_grdnt_img_rqsts;

    bool delete_wine_dir_on_exit = false;
    bool delete_wine_prefix_on_exit = false;

    bool using_wine;
    bool using_timeout;
    bool setting_env_vars;

    bool is_initialised;
    std::string initialisation_error_msgs;

    std::string run_command;
    boost::filesystem::path run_bat_file;
    boost::filesystem::path save_bat_file;
    boost::filesystem::path run_sh_file;
    boost::filesystem::path save_sh_file;
    std::stringstream run_bat_file_contents;
    std::stringstream save_bat_file_contents;
    std::stringstream run_sh_file_contents;
    std::stringstream save_sh_file_contents;
    
public:
    GeonamicaOptimiser( GeonamicaPolicyParameters & _params);

    ~GeonamicaOptimiser();

    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars);

    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
               boost::filesystem::path & save_path);

    ProblemDefinitionsSPtr getProblemDefinitions();

private:

    //Copies entire directory - so that each geoproject is running in a different directory.
    bool
    copyDir(   boost::filesystem::path const & source,
               boost::filesystem::path const & destination );

    //Copies entire directory - so that each geoproject is running in a different directory.
    bool
    copyFilesInDir(
        boost::filesystem::path const & source,
        boost::filesystem::path const & destination
    );

    void
    runGeonamica(std::ofstream & logging_file, bool do_save);

    template<typename T> void
    setXPathDVValue(pugi::xml_document & doc, XPathDV& xpath_details, T new_value);

    template <typename T> void
    setAllChildValuesOfXMLNode(pugi::xml_document & doc, std::string  xpath_query, T new_value);

    void
    addPointElement(pugi::xml_node & node, double x_val, double & y_val);

//    void
//    setSplineCurveProportional(pugi::xml_document & doc, std::string xpath_query, std::vector<std::vector<double> > &
//    base_spline, double factor);

    void
    removeOldOutputs();

    std::vector<double>
    calcObjectives(std::ofstream & logging_file, const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars);

    void
    calculate(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
              boost::filesystem::path save_path = "no_path", boost::filesystem::path _logfile = "unspecified");

    double
    sumMap(blink::raster::gdal_raster<double> & map);

    boost::optional<double>
    sumMap(const boost::filesystem::path &map_path_year, int recurse_depth = 0);

    bool
    makeZonalMap(const std::vector<int> &int_decision_vars, int recurse_depth = 0);

    void
    makeZonalMap(std::vector<int>::const_iterator first_zone_dv,
                 blink::raster::gdal_raster<int> & zonal_map);

    template <typename T> void
    saveMap(const SaveMapDetails &save_details, const boost::filesystem::path &map_path, const boost::filesystem::path &save_path, int recurse_depth = 0) const;

    template <typename T> void
    saveMap(blink::raster::gdal_raster<T> & map, const boost::filesystem::path save_path, const SaveMapDetails & save_details) const;

    void
    makeWorseObjValues(std::vector<double> & objectives);

    void
    saveMapsAndObjAndConstraints(const boost::filesystem::path &save_path, int replicate_number, std::vector<double> & obj_vals);
};


//TODO:
// 1. manipulate geoproject with seed numbers
// 2. Masks for objective maps to exlcude areas of summing up


#endif /* ZonalPolicyOptimiser_hpp */
