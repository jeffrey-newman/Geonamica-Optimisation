//
//  ZonalPolicyUtility.hpp
//  GeonamicaOptimiser
//
//  Created by a1091793 on 5/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef ZonalPolicyUtility_hpp
#define ZonalPolicyUtility_hpp

#include <string>
#include "Pathify.hpp"
#include "ProblemDefinitions.hpp"
#include <boost/program_options.hpp>

struct ZonalPolicyParameters
{
    std::string timout_cmd; //cmd to run everything through another program which kills model on timer, incase it gets stuck/spins/hangs
    std::string wine_cmd; // cmd to run geonamica model through wine, emulation for running on linux or mac.
    std::string geonamica_cmd;  // path to geonamicaCmd.exe. Usually UNIX or Wine path both should work Z://PATH/geonamica.exe")
    bool with_reset_and_save;
    CmdLinePaths template_project_dir; // path to template project
    CmdLinePaths working_dir;
    CmdLinePaths wine_prefix_path;
    std::string wine_geoproject_disk_drive;
    bool set_prefix_path;
    std::string windows_env_var;
    CmdLinePaths wine_drive_path;
//    std::string wine_drive_letter;
    std::string wine_working_dir;
    std::string rel_path_geoproj; // relative path of geoproject file from geoproject directory (head/root directory)
    std::string rel_path_zonal_map; // relative path of zonal policy map layer that is being optimised relative to (head/root directory.)
    std::string zonal_map_classes; // values that are valid in the zonal map. The optimisation specifes one fo these for each of the delinatied araas in zones_delineation_map.
    std::vector<std::string> rel_path_obj_maps; // relative paths of objectives maps that we are maximising/minimising relative to (head/root directory)
    std::vector<std::string> objectives_plugins; // vector of objective plugins....
    std::vector<std::string> xpath_dvs;
//    std::vector<std::string> min_or_max_str;
    std::vector<MinOrMaxType> min_or_max; //vector of whether the objectives in the maps above are minimised or maximised.
    std::string rel_path_log_specification_obj;  // relative path of logging file from geoproject directory (head/root directory)  - should be in wine format.
    std::string rel_path_log_specification_save;  // relative path of logging file from geoproject directory (head/root directory)  - should be in wine format.
    std::string rel_path_zones_delineation_map;  // relative path of a map which delineates the project area into regions wherein zonal policiy is optimised.
    std::vector<std::string> save_maps;
    bool is_logging = false;
    int replicates = 10;
    int pop_size; // For the GA
    int max_gen_hvol;  // Termination condition for the GA
    int max_gen;
    int save_freq;
    CmdLinePaths save_dir;
    int evaluator_id = 0;
    std::vector<int> rand_seeds { 1000,1001,1002,1003,1004,1005,1006,1007,1008,1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020 };
    CmdLinePaths restart_pop_file;
//    int year_start_saving;
//    int year_end_saving;
//    std::vector<int> years_metric_calc;
//    std::vector<int> years_save;
//    int discount_start_year;
//    double discount_rate;
    
};

class LoadParameters
{

public:
    LoadParameters();

    int
    processOptions(int argc, char * argv[], ZonalPolicyParameters & _params);

    int
    processOptions(std::string filepath, ZonalPolicyParameters & _params);

    int
    saveOptions(std::string filepath, ZonalPolicyParameters & _params);

private:
    boost::program_options::options_description desc;
    boost::filesystem::path deafult_working_dir;
    boost::program_options::variables_map vm;
    ZonalPolicyParameters params;

};

#endif /* ZonalPolicyUtility_hpp */
