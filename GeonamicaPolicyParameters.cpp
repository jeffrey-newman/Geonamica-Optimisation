//
// Created by a1091793 on 18/8/17.
//

//
//  ZonalPolicyUtility.hpp
//  GeonamicaOptimiser
//
//  Created by a1091793 on 5/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//


#include "GeonamicaPolicyParameters.hpp"

#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "Pathify.hpp"
#include "UserHomeDirectory.hpp"

std::pair<std::string, std::string> at_option_parser(std::string const&s)
{
    if ('@' == s[0])
        return std::make_pair(std::string("cfg-file"), s.substr(1));
    else
        return std::pair<std::string, std::string>();
}

LoadParameters::LoadParameters() :
    desc("Allowed options")
{
    namespace po = boost::program_options;
    try
    {

        boost::filesystem::path deafult_working_dir =
                boost::filesystem::path(userHomeDir()) / ".geonamicaZonalOpt/working_dir";


        desc.add_options()
                ("help,h", "produce help message")
                ("timeout-cmd,u", po::value<std::string>(&params.timout_cmd)->default_value("no_timeout"),
                 "[optional] excutable string that will run the timeout cmd --- to run everything through another program which kills model on timer, incase it gets stuck/spins/hangs")
                ("wine-cmd,f", po::value<std::string>(&params.wine_cmd)->default_value("no_wine"),
                 "[optional] path to wine (emulator) excutable ")
                ("geonamica-cmd,m", po::value<std::string>(&params.geonamica_cmd),
                 "excutable string that will run the geonamica model --- without command flags/arguments (like Z://PATH/geonamica.exe\")")
                ("with-reset-and-save", po::value<bool>(&params.with_reset_and_save)->default_value(true),
                 "whether the --save flag is used with respective --reset for each simulation")
                ("template,t", po::value<std::string>(&params.template_project_dir.first),
                 "path to template geoproject directory")
                ("working-dir,d",
                 po::value<std::string>(&params.working_dir.first)->default_value(deafult_working_dir.string()),
                 "path of directory for storing temp files during running")
                ("wine-prefix-path,w", po::value<std::string>(&params.wine_prefix_path.first)->default_value("na"),
                 "Path to the wine prefix to use. Subfolder should contain dosdevices. To use default in home drive, specify <use_home_drive> to generate new prefix use <generate>")
                ("set-prefix-env-var", po::value<bool>(&params.set_prefix_path)->default_value(true),
                 "If using crossover, the prefix is set as part of wine command, and so can be switched off.")
                //            ("wine-drive-path,f", po::value<std::string>(&params.wine_prefix_path.first)->default_value("do not test"), "Path of root directory of wine drive")
                //            ("wine-drive-letter,g", po::value<std::string>(&params.wine_drive_letter), "Letter of drive to make symlink for - i.e. C for 'C:' or Z for 'Z:' etc ")
                //    ("wine-work-dir,w", po::value<std::string>(&params.wine_working_dir), "path to working directory (working-dir,d), but in wine path format - e.g. Z:\\path\\to\\working\\dir")

                ("geoproj-file,g", po::value<std::string>(&params.rel_path_geoproj),
                 "name of geoproject file (without full path), relative to template geoproject directory. Needs to be in top level at the moment")
                ("log-file-objectives,l", po::value<std::string>(&params.rel_path_log_specification_obj),
                 "path of the log settings xml file (relative to template geoproject directory in in wine format for use during optimisation process)")
                ("log-file-save,n",
                 po::value<std::string>(&params.rel_path_log_specification_save)->default_value("unspecified"),
                 "path of the log settings xml file (relative to template geoproject directory in in wine format), but for saving outputs")
                ("replicates,i", po::value<int>(&params.replicates)->default_value(10),
                 "Number of times to rerun Metronamica to account for stochasticity of model for each objective function evaluation")

                ("obj-maps,o", po::value<std::vector<std::string> >(&params.rel_path_obj_maps)->multitoken(),
                 "relative paths wrt template geoproject directory of objective maps")
//        ("min-or-max,n",po::value<std::vector<std::string> >(&params.min_or_max_str)->multitoken(), "whether the aggregated value in the obj-maps are to be minimised (specify MIN) or maximised (specify MAX)")
                ("obj-plugins", po::value<std::vector<std::string> >(&params.objectives_plugins)->multitoken(), " paths to objective modules and constructure string. FORMAT: [\"PATH\"]:[\"Constructor_string\"]")
                ("zonal-maps,z", po::value<std::string>(&params.rel_path_zonal_map)->default_value("no_zonal_dvs"),
                 "name of zonal map (without full path), relative to template geoproject directory. This needs to be GDAL create writable, so NOT ASCII grid format")
                ("zone-delineation,e",
                 po::value<std::string>(&params.rel_path_zones_delineation_map)->default_value("no_zonal_dvs"),
                 "name of zonal delineation map (without full path), relative to template geoproject directory")
                ("xpath-dv,j", po::value<std::vector<std::string> >(&params.xpath_dvs)->multitoken(),
                 "xpath for decision variable, See documentation for format")

                ("is-logging,s", po::value<bool>(&params.is_logging)->default_value(false),
                 "TRUE or FALSE whether to log the evaluation")
                ("save-dir,v", po::value<std::string>(&params.save_dir.first)->default_value(
                        boost::filesystem::current_path().string()),
                 "path of the directory for writing results and outputs to")
                ("save-map,k", po::value<std::vector<std::string> >(&params.save_maps)->multitoken(),
                 "relative path to geoproject directory for maps to save when optimisation completes. Format: [CATEGORISED/LINEAR_GRADIENT]:LEGEND=\"[legend_specification_file_relative_to_geoproject]\":PATH=[\"path_of_map_relative_to_geoproject\"]:DIFF=[\"opt_path_of_differencing_map_relative_to_geoproject\"]:SAVE_AS=[\"file_name\"]")

                ("pop-size,p", po::value<int>(&params.pop_size)->default_value(415), "Population size of the NSGAII")
                ("max-gen-no-hvol-improve,x", po::value<int>(&params.max_gen_hvol)->default_value(50),
                 "maximum generations with no improvement in the hypervolume metric - terminaation condition")
                ("max-gen,y", po::value<int>(&params.max_gen)->default_value(500),
                 "Maximum number of generations - termination condition")
                ("save-freq,q", po::value<int>(&params.save_freq)->default_value(1),
                 "how often to save first front, hypervolume metric and population")

                ("reseed,r", po::value<std::string>(&params.restart_pop_file.first)->default_value("no_seed"),
                 "File with saved population as initial seed population for GA")
                ("year-start,a", po::value<int>(&params.year_start),
                 "Start year for objective map logging - only valid if objective logging file stem is only given and not full filename")
                ("year-end,b", po::value<int>(&params.year_end),
                 "End year for objective map logging - only valid if objective logging file stem is only given and not full filename")
                ("discount-rate,d", po::value<double>(&params.discount_rate)->default_value(0.0),
                 "discount rate for objectives"
                         "es (applies to all of them)")
                ("cfg-file,c", po::value<std::string>(), "can be specified with '@name', too");
    }
        catch(std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
//            return 1;
        }
        catch(...)
        {
            std::cerr << "Unknown error!" << "\n";
//            return 1;
        }
}

int
LoadParameters::processOptions(int argc, char * argv[], ZonalPolicyParameters & _params)
{

    namespace po = boost::program_options;
        try
        {
            po::store(po::command_line_parser(argc, argv).options(desc).extra_parser(at_option_parser).run(), vm);

            if (vm.count("help"))
            {
                std::cout << desc << "\n";
                return 1;
            }

            if (vm.count("cfg-file")) {
                // Load the file and tokenize it
                std::ifstream ifs(vm["cfg-file"].as<std::string>().c_str());
                if (!ifs) {
                    std::cout << "Could not open the cfg file\n";
                    return 1;
                }
                po::store(po::parse_config_file(ifs, desc), vm);
            }

            po::notify(vm);
            //    pathify(params.geonamica_cmd); //.second = boost::filesystem::path(metro_exe.first);
            pathify_mk(params.working_dir);
            pathify(params.template_project_dir);
            //pathify(params.log_dir);
            pathify_mk(params.save_dir);
            if (params.restart_pop_file.first != "no_seed")
            {
                pathify(params.restart_pop_file);
            }
        }
        catch(po::error& e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return 1;
        }

    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown error!" << "\n";
        return 1;
    }
    _params = this->params;

    return (0);

}

int
LoadParameters::processOptions(std::string filepath, ZonalPolicyParameters & _params)
{

    namespace po = boost::program_options;
    try
    {


           // Load the file and tokenize it
            std::ifstream ifs(filepath.c_str());
            if (!ifs) {
                std::cout << "Could not open the cfg file\n";
                return 1;
            }
            po::store(po::parse_config_file(ifs, desc), vm);

        po::notify(vm);
        pathify_mk(params.working_dir);
        pathify(params.template_project_dir);
        //pathify(params.log_dir);
        pathify_mk(params.save_dir);
        if (params.restart_pop_file.first != "no_seed")
        {
            pathify(params.restart_pop_file);
        }
    }
    catch(po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown error!" << "\n";
        return 1;
    }
    _params = this->params;

    return (0);

}

int LoadParameters::saveOptions(std::string filepath, ZonalPolicyParameters &_params)
{
    std::ofstream ofs(filepath.c_str());
    if (ofs.is_open())
    {
        ofs << "timeout-cmd = " << _params.timout_cmd << "\n";
        ofs << "wine-cmd = " << _params.wine_cmd << "\n";
        ofs << "geonamica-cmd = " << _params.geonamica_cmd << "\n";
        ofs << "with-reset-and-save = " << _params.with_reset_and_save << "\n";
        ofs << "template = " << _params.template_project_dir.first << "\n";
        ofs << "working-dir = " << _params.working_dir.first << "\n";
        ofs << "wine-prefix-path = " << _params.wine_prefix_path.first << "\n";
        ofs << "set-prefix-env-var = " << _params.set_prefix_path << "\n";
        ofs << "geoproj-file = " << _params.rel_path_geoproj << "\n";
        ofs << "log-file-objectives = " << _params.rel_path_log_specification_obj << "\n";
        ofs << "log-file-save = " << _params.rel_path_log_specification_save << "\n";
        ofs << "replicates = " << _params.replicates << "\n";
        std::for_each(_params.rel_path_obj_maps.begin(), _params.rel_path_obj_maps.end(), [&ofs] (std::string &
        rel_path_obj_map) {
            ofs << "obj-maps = " << rel_path_obj_map << "\n";
                });
        std::for_each(_params.objectives_plugins.begin(), _params.objectives_plugins.end(), [&ofs] (std::string &
        objective_plugin_string) {
            ofs << "obj-plugins = " << objective_plugin_string << "\n";
        });
        ofs << "zonal-maps = " << _params.rel_path_zonal_map << "\n";
        ofs << "zone-delineation = " << _params.rel_path_zones_delineation_map << "\n";
        std::for_each(_params.xpath_dvs.begin(), _params.xpath_dvs.end(), [&ofs] (std::string &
        xpath_dvs_str) {
            ofs << "xpath-dv = " << xpath_dvs_str << "\n";
        });
        ofs << "is-logging = " << _params.is_logging << "\n";
        ofs << "save-dir = " << _params.save_dir.first << "\n";
        std::for_each(_params.save_maps.begin(), _params.save_maps.end(), [&ofs] (std::string &
        save_map_spec) {
            ofs << "save-map = " << save_map_spec << "\n";
        });
        ofs << "pop-size = " << _params.pop_size << "\n";
        ofs << "max-gen-no-hvol-improve = " << _params.max_gen_hvol << "\n";
        ofs << "max-gen = " << _params.max_gen << "\n";
        ofs << "save-freq = " << _params.save_freq << "\n";
        ofs << "reseed = " << _params.restart_pop_file.first << "\n";
        ofs << "year-start = " << _params.year_start << "\n";
        ofs << "year-end = " << _params.year_end << "\n";
        ofs << "discount-rate = " << _params.discount_rate << "\n";

    }

    ofs.close();
    return 0;
}

