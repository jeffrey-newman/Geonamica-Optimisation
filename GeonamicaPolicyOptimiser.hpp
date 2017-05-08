//
//  ZonalPolicyOptimiser.hpp
//  GeonamicaOptimiser
//
//  Created by a1091793 on 4/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef ZonalPolicyOptimiser_hpp
#define ZonalPolicyOptimiser_hpp

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <tuple>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp> // include Boost, a C++ library
#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include <boost/timer/timer.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/bind.hpp>
#include "ColourMapperParsers.h"
#include "Evaluation.hpp"
#include <blink/raster/utility.h>
#include <blink/raster/utility.h> // To open rasters
#include <blink/iterator/zip_range.h>
#include <unordered_set>
#include <pugixml.hpp>

#include <stdio.h>

#include "GeonamicaPolicyUtility.hpp"

class GeonamicaOptimiser : public ObjectivesAndConstraintsBase
{
private:
    
    ZonalPolicyParameters params;
    
    
    boost::filesystem::path working_project;
    std::string wine_working_project;
    
    boost::filesystem::path working_logging;
    std::string wine_working_logging;
    
    boost::filesystem::path zonal_map_path;

//    std::vector<boost::filesystem::path> obj_map_paths;
    std::vector<boost::filesystem::path> obj_map_paths_minisation;
    std::vector<boost::filesystem::path> obj_map_paths_maximisation;
    
    boost::filesystem::path zones_delineation_map_path;
    blink::raster::gdal_raster<int> zones_delineation_map;
    boost::optional<int> zones_delineation_no_data_val;
    std::unordered_set<int> zones;
    
    boost::filesystem::path logfile;
    boost::filesystem::path previous_logfile;

//    std::string temp_dir_template;
    
    
//    int analysisNum;
    int eval_count;
    
    int num_objectives;
    int num_constraints = 0;

    int min_zonal_dv_values  = 0; //lower bounds
    int max_zonal_dv_values = 3; // upper bound of x
    // Decision variable mapping:
    // 0 -- 0 Strictly restrict
    // 1 -- 0.5 Weakly restrict
    // 2 -- 1 Allow
    // 3 -- 1.5 Stimulate
    std::map<int, float> zone_policies = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    std::vector<int> zone_policies_vec = {1,2,3,4};

    std::vector<std::string> int_dv_xpaths;
    std::vector<std::string> real_dv_xpaths;

    std::vector<int> int_lowerbounds;
    std::vector<int> int_upperbounds;
    std::vector<double > real_lowerbounds;
    std::vector<double > real_upperbounds;

    ProblemDefinitionsSPtr prob_defs;
    std::pair<std::vector<double>, std::vector<double> > objectives_and_constraints;


    std::vector<std::tuple<boost::filesystem::path, boost::shared_ptr<ColourMapperClassified>,     boost::shared_ptr<MagickWriterClassified>, std::string > > classified_img_rqsts;
    std::vector<std::tuple<boost::filesystem::path, boost::shared_ptr<ColourMapperLinearGradient>, boost::shared_ptr<MagickWriterLinearGradient>, std::string > > lin_grdnt_img_rqsts;

    bool delete_wine_dir_on_exit = false;
    bool delete_wine_prefix_on_exit = false;

    bool using_wine;
    bool using_timeout;
    
    //Copies entire directory - so that each geoproject is running in a different directory.
    bool copyDir(   boost::filesystem::path const & source,
                    boost::filesystem::path const & destination )
    {
//        std::cout << "Copying " << source << " to " << destination << std::endl;
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(!fs::exists(source) || !fs::is_directory(source))
            {
                std::cerr << "Source directory " << source.string()
                          << " does not exist or is not a directory." << '\n';
                return false;
            }
            if(fs::exists(destination))
            {
                std::cerr << "Destination directory " << destination.string()
                          << " already exists." << '\n';
                return false;
            }
            // Create the destination directory
            fs::copy_directory(source, destination);
            if(!fs::exists(destination) || !fs::is_directory(destination))
            {
                std::cerr << "Unable to create destination directory"
                          << destination.string() << '\n';
                return false;
            }
        }
        catch(fs::filesystem_error const & e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        // Iterate through the source directory
        for(
                fs::directory_iterator file(source);
                file != fs::directory_iterator(); ++file
                )
        {

            try
            {
                fs::path current(file->path());
//                std::cout << "descending into: " << current << std::endl;
                if(fs::is_directory(current))
                {
                    // Found directory: Recursion
                    if(!copyDir(current, destination / current.filename()))
                    {
                        return false;
                    }
                }
                else
                {
                    // Found file: Copy
//                    std::cout << "Copying " << current << " to " << destination / current.filename() << std::endl;
                    fs::copy_file(current, destination / current.filename());
                }
            }
            catch(fs::filesystem_error const & e)
            {
                std:: cerr << e.what() << '\n';
            }
        }
        return true;
    }

    //Copies entire directory - so that each geoproject is running in a different directory.
    bool copyFilesInDir(
            boost::filesystem::path const & source,
            boost::filesystem::path const & destination
    )
    {
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(!fs::exists(source) || !fs::is_directory(source))
            {
                std::cerr << "Source directory " << source.string()
                          << " does not exist or is not a directory." << '\n';
                return false;
            }
            if(!fs::exists(destination) || !fs::is_directory(destination))
            {
                std::cerr << "Destination directory " << destination.string()
                          << " does not exist or is not a directory." << '\n';
                return false;
            }

        }
        catch(fs::filesystem_error const & e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        // Iterate through the source directory
        for(
                fs::directory_iterator file(source);
                file != fs::directory_iterator(); ++file
                )
        {
            try
            {
                fs::path current(file->path());
                if(!(fs::is_directory(current)))  // Should we be checking for other things?
                {
                    // Found file: Copy
                    fs::copy_file(current, destination / current.filename());
                }
            }
            catch(fs::filesystem_error const & e)
            {
                std:: cerr << e.what() << '\n';
            }
        }
        return true;
    }

//    //Copies entire directory - so that each geoproject is running in a different directory.
//    bool copyDirContents(
//            boost::filesystem::path const & source,
//            boost::filesystem::path const & destination
//    )
//    {
//        namespace fs = boost::filesystem;
//        try
//        {
//            // Check whether the function call is valid
//            if(!fs::exists(source) || !fs::is_directory(source))
//            {
//                std::cerr << "Source directory " << source.string()
//                          << " does not exist or is not a directory." << '\n';
//                return false;
//            }
//            if(!fs::exists(destination) || !fs::is_directory(destination))
//            {
//                std::cerr << "Destination directory " << destination.string()
//                          << " does not exist or is not a directory." << '\n';
//                return false;
//            }
//        }
//        catch(fs::filesystem_error const & e)
//        {
//            std::cerr << e.what() << '\n';
//            return false;
//        }
//        // Iterate through the source directory
//        for(fs::directory_iterator file(source); file != fs::directory_iterator(); ++file)
//        {
//            try
//            {
//                fs::path current(file->path());
//                if(fs::is_directory(current))
//                {
//                    // Found directory: Recursion
//                    if(!copyDir(current, destination / current.filename()))
//                    {
//                        return false;
//                    }
//                }
//                else
//                {
//                    // Found file: Copy
//                    fs::copy_file(current, destination / current.filename());
//                }
//            }
//            catch(fs::filesystem_error const & e)
//            {
//                std:: cerr << e.what() << '\n';
//            }
//        }
//        return true;
//    }
    
    
public:
    GeonamicaOptimiser( ZonalPolicyParameters & _params)
    :
    params(_params),
    eval_count(0),
    num_objectives(0)
    {


        // Set up wine prefixes and wine paths.
        using_wine = false;
        if (params.wine_cmd != "no_wine")
        {
            using_wine = true;
            // Configure Wine prefix tp use.
            if (params.wine_prefix_path.first == "use_home_path")
            {
                params.wine_prefix_path.second = boost::filesystem::path(userHomeDir()) / ".wine";
                params.wine_prefix_path.first = params.wine_prefix_path.second.string();
            } else if (params.wine_prefix_path.first.substr(0, 8) == "generate")
            {
                std::string prefix_template =
                        "Metro_Cal_OF_worker" + std::to_string(params.evaluator_id) + "_wine_prefix_%%%%-%%%%";
                params.wine_prefix_path.second = boost::filesystem::unique_path(
                        params.working_dir.second / prefix_template);
                params.wine_prefix_path.first = params.wine_prefix_path.second.string();
                std::stringstream cmd;
                cmd << "WINEPREFIX=" << params.wine_prefix_path.second.string().c_str() << " " << params.wine_cmd << " winecfg";
                int return_val = system(cmd.str().c_str());
                this->delete_wine_prefix_on_exit = true;

                //Copy model into prefix
                boost::filesystem::path template_geonamica_binary_root = params.wine_prefix_path.first.substr(9);
                boost::filesystem::path copy_geonamica_binary_root =
                        params.wine_prefix_path.second / "drive_c/Program Files (x86)/Geonamica";
                copyDir(template_geonamica_binary_root, copy_geonamica_binary_root);


            } else if (params.wine_prefix_path.first.substr(0, 4) == "copy")
            {
                std::string prefix_copy_template =
                        "Metro_Cal_OF_worker" + std::to_string(params.evaluator_id) + "_wine_prefix_%%%%-%%%%";
                boost::filesystem::path prefix_copied_path = boost::filesystem::unique_path(
                        params.working_dir.second / prefix_copy_template);
                boost::filesystem::path prefix_template_path = params.wine_prefix_path.first.substr(5);
                boost::filesystem::create_directories(prefix_copied_path);
                copyFilesInDir(prefix_template_path, prefix_copied_path);

                boost::filesystem::path drive_c_copied_path = prefix_copied_path / "drive_c";
                boost::filesystem::path drive_c_template_path = prefix_template_path / "drive_c";
                copyDir(drive_c_template_path, drive_c_copied_path);

                boost::filesystem::copy_directory(prefix_template_path / "dosdevices",
                                                  prefix_copied_path / "dosdevices");
                boost::filesystem::path drive_c_link = prefix_copied_path / "dosdevices/c:";
                boost::filesystem::create_directory_symlink(drive_c_copied_path, drive_c_link);
                boost::filesystem::path drive_z_link = prefix_copied_path / "dosdevices/z:";
                boost::filesystem::create_directory_symlink(boost::filesystem::path("/"), drive_z_link);

                params.wine_prefix_path.second = prefix_copied_path;
                params.wine_prefix_path.first = params.wine_prefix_path.second.string();
                this->delete_wine_prefix_on_exit = true;
            } else
            {
                pathify_mk(params.wine_prefix_path); //although should really already exist....
            }

            // Check dosdevices path exists.
            params.wine_drive_path.second = params.wine_prefix_path.second / "dosdevices";
            params.wine_drive_path.first = params.wine_drive_path.second.string();
            if (!(boost::filesystem::exists(params.wine_drive_path.second)))
            {
                std::cout << "Could not find dosdevices in " << params.wine_prefix_path.second
                          << " Is wine installed?\n";

            }

            // Copy project directory into working directory
            std::string temp_dir_template = "Metro_Cal_OF_worker" + std::to_string(params.evaluator_id) + "_%%%%-%%%%";
            params.working_dir.second = boost::filesystem::unique_path(params.working_dir.second / temp_dir_template);
            params.working_dir.first = params.working_dir.second.string();
//            temp_dir_template = params.working_dir.second.filename().string();
            copyDir(params.template_project_dir.second, params.working_dir.second);
//            params.wine_working_dir = params.wine_working_dir + "\\" + temp_dir_template;


            // Create new dosdevice drive to working geoproject file directory.
            std::vector<std::string> drive_options = {"m:", "n:", "o:", "p:", "q:", "r:", "s:", "t:", "u:", "v:", "w:",
                                                      "x:", "y:", "l:", "a:", "b:"};

            BOOST_FOREACH(std::string &drive_option, drive_options)
                        {
                            boost::filesystem::path symlinkpath_ext = params.wine_drive_path.second / drive_option;
                            //Check if symbolic link for wine J: exists.
                            boost::filesystem::file_status lnk_status = boost::filesystem::symlink_status(
                                    symlinkpath_ext);
                            if (!(boost::filesystem::is_symlink(lnk_status)) ||
                                !(boost::filesystem::exists(symlinkpath_ext)))
                            {
                                boost::filesystem::create_directory_symlink(params.working_dir.second, symlinkpath_ext);
//                                params.wine_drive_letter = drive_option;
                                params.wine_working_dir = drive_option;
                                params.wine_drive_path.second = params.wine_drive_path.second / drive_option;
                                delete_wine_dir_on_exit = true;
                                break;
                            }
                            if (drive_option == "b:")
                                std::cerr << "Could not make a symlink to the working drive for winedrive.\n";
                        }
        }

        using_timeout = false;
        if (params.timout_cmd != "no_timeout")
        {
            using_timeout = true;
        }

        // get paths of important files in working directory.
        working_project = params.working_dir.second / params.rel_path_geoproj;
        wine_working_project = params.wine_working_dir + "\\" + params.rel_path_geoproj;
        zonal_map_path = params.working_dir.second / params.rel_path_zonal_map;


        namespace qi = boost::spirit::qi;
        namespace ph = boost::phoenix;

        //Object to hold objectives and constraints.
        objectives_and_constraints = std::pair<std::vector<double>, std::vector<double> >(std::piecewise_construct, std::make_tuple(0), std::make_tuple(num_constraints));

        // Set up formulation of optimisation problem.
        // Work out maximisatino or minisation for objectives for maps, and the path to the maps]
        qi::rule<std::string::iterator, std::string()> string_parser = (+qi::char_);   //[_val = _1]
//        string_parser.name("string_parser");
//        qi::debug(string_parser);
        std::vector<std::string> minimisation_obj_paths;
        std::vector<std::string> maximisation_obj_paths;
        auto obj_map_parser =  (qi::no_case[(( qi::lit("MAXIMISE") | qi::lit("MAXIMIZE") | qi::lit("MAX")))] >> qi::lit(":") >> string_parser[ph::push_back(ph::ref(maximisation_obj_paths), qi::_1)])
                            | (qi::no_case[((qi::lit("MINIMISE") | qi::lit("MINIMIZE") | qi::lit("MIN"))) ] >> qi::lit(":") >> string_parser[ph::push_back(ph::ref(minimisation_obj_paths), qi::_1)]);

        BOOST_FOREACH(std::string & rel_path, params.rel_path_obj_maps)
        {
            boost::spirit::qi::parse(rel_path.begin(), rel_path.end(), obj_map_parser);
        }
        BOOST_FOREACH(std::string & min_rel_path, minimisation_obj_paths)
                    {
                        obj_map_paths_minisation.push_back(params.working_dir.second / min_rel_path);
                        params.min_or_max.push_back(MINIMISATION);
                        objectives_and_constraints.first.push_back(std::numeric_limits<double>::max());
                        num_objectives++;
                    }
        BOOST_FOREACH(std::string & max_rel_path, maximisation_obj_paths)
                    {
                        obj_map_paths_maximisation.push_back(params.working_dir.second / max_rel_path);
                        params.min_or_max.push_back(MAXIMISATION);
                        objectives_and_constraints.first.push_back(std::numeric_limits<double>::min());
                        num_objectives++;
                    }

        if (obj_map_paths_minisation.size() > 0 || obj_map_paths_maximisation.size() > 0 )
        {
            params.min_or_max.push_back(MINIMISATION);  // For minimising the area which has a restrictive (and stimulating?) zonal policies.
            objectives_and_constraints.first.push_back(std::numeric_limits<double>::max());
            num_objectives++;
        }

        zones_delineation_map_path = params.working_dir.second / params.rel_path_zones_delineation_map;
        working_logging = params.working_dir.second / params.rel_path_log_specification_obj;
        wine_working_logging = params.wine_working_dir + "\\" + params.rel_path_log_specification_obj;

        // Load maps into memory
        zones_delineation_map = blink::raster::open_gdal_raster<int>(zones_delineation_map_path, GA_ReadOnly);
        // Calculate number of zones (this will be equal to the number of decision variables related to the zonal policy)
        auto zip = blink::iterator::make_zip_range(std::ref(zones_delineation_map));
        zones_delineation_no_data_val = zones_delineation_map.noDataVal();
        for (auto i : zip)
        {
            int val_i = std::get<0>(i);
            if (zones_delineation_no_data_val)
            {
                if (val_i != zones_delineation_no_data_val.get())
                {
                    zones.emplace(val_i);
                }
            }
            else
            {
                zones.emplace(val_i);
            }
//            if (val != no_data_val)
        }

        int_lowerbounds.resize(zones.size(), min_zonal_dv_values);
        int_upperbounds.resize(zones.size(), max_zonal_dv_values);


        auto xpath_policy_dv_parser =  ( qi::lit("INT") >> qi::lit("(") >> qi::int_[ph::push_back(ph::ref(int_lowerbounds), qi::_1)]
                                                        >>  qi::lit(",") >> qi::int_[ph::push_back(ph::ref(int_upperbounds), qi::_1)]  >>  qi::lit(")")
                                                        >>  string_parser[ph::push_back(ph::ref(int_dv_xpaths), qi::_1)])
                                        | ( qi::lit("REAL") >> qi::lit("(") >> qi::int_[ph::push_back(ph::ref(real_lowerbounds), qi::_1)]
                                                        >>  qi::lit(",") >> qi::int_[ph::push_back(ph::ref(real_upperbounds), qi::_1)] >>  qi::lit(")")
                                                        >>  string_parser[ph::push_back(ph::ref(this->real_dv_xpaths), qi::_1)]);


        BOOST_FOREACH(std::string & xpath, params.xpath_dvs)
                    {
                        boost::spirit::qi::parse(xpath.begin(), xpath.end(), xpath_policy_dv_parser);
                        num_objectives++;
//            obj_map_paths.push_back(params.working_dir.second / rel_path);
                    }

//        typedef std::tuple<std::string, std::string, std::string> StringTuple;
        typedef std::vector<std::string> StringTuple;
        std::vector<StringTuple > classified_img_rqsts_tmp;
        std::vector<StringTuple> lin_grdnt_img_rqsts_tmp;
        qi::rule<std::string::iterator, std::string()> string_parser_quote_delimited = qi::lit("\"") >> +(qi::char_ - "\"") >> qi::lit("\"");   //[_val = _1]
        string_parser_quote_delimited.name("string_quote_delimited_parser");
//        qi::debug(string_parser_quote_delimited);
        qi::rule<std::string::iterator, StringTuple()> catgeorised_save_map_rule = qi::no_case[qi::lit("CATEGORISED") | qi::lit("CAT")] >> qi::lit(":") >> string_parser_quote_delimited >> qi::lit(":") >> string_parser_quote_delimited >> qi::lit(":") >> string_parser_quote_delimited;
        qi::rule<std::string::iterator, StringTuple()> line_grad_save_map_rule = qi::no_case[qi::lit("LINEAR_GRADIENT") | qi::lit("LIN_GRAD")] >> qi::lit(":") >> string_parser_quote_delimited >> qi::lit(":") >> string_parser_quote_delimited >> qi::lit(":") >> string_parser_quote_delimited;
        auto save_maps_parser = catgeorised_save_map_rule[ph::push_back(ph::ref(classified_img_rqsts_tmp), qi::_1)] | line_grad_save_map_rule[ph::push_back(ph::ref(lin_grdnt_img_rqsts_tmp), qi::_1)];
        BOOST_FOREACH(std::string & save_map, params.save_maps)
                    {
                        boost::spirit::qi::parse(save_map.begin(), save_map.end(),  save_maps_parser);
                    }
        BOOST_FOREACH(StringTuple & cat_save_map_pair, classified_img_rqsts_tmp)
                    {
//                        boost::shared_ptr<ColourMapperClassified> classified_clr_map =  parseColourMapClassified(params.working_dir.second / std::get<0>(cat_save_map_pair));
                        boost::shared_ptr<ColourMapperClassified> classified_clr_map =  parseColourMapClassified(params.working_dir.second / (cat_save_map_pair[0]));
                        boost::shared_ptr<MagickWriterClassified> classified_map_prntr(new MagickWriterClassified(*classified_clr_map));
//                        classified_img_rqsts.push_back(std::make_tuple(params.working_dir.second / std::get<1>(cat_save_map_pair), classified_clr_map, classified_map_prntr, std::get<2>(cat_save_map_pair)));
                        classified_img_rqsts.push_back(std::make_tuple(params.working_dir.second / (cat_save_map_pair[1]), classified_clr_map, classified_map_prntr, (cat_save_map_pair[2])));
                    }
        BOOST_FOREACH(StringTuple & lin_grad_save_map_pair, lin_grdnt_img_rqsts_tmp)
                    {
//                        boost::shared_ptr<ColourMapperLinearGradient> lin_grad_clr_map = parseColourMapLinearGradient(params.working_dir.second / std::get<0>(lin_grad_save_map_pair));
                        boost::shared_ptr<ColourMapperLinearGradient> lin_grad_clr_map = parseColourMapLinearGradient(params.working_dir.second / (lin_grad_save_map_pair[0]));
                        boost::shared_ptr<MagickWriterLinearGradient> lin_grad_map_prntr(new MagickWriterLinearGradient(*lin_grad_clr_map));
//                        lin_grdnt_img_rqsts.push_back(std::make_tuple(params.working_dir.second / std::get<1>(lin_grad_save_map_pair), lin_grad_clr_map, lin_grad_map_prntr, std::get<2>(lin_grad_save_map_pair)));
                        lin_grdnt_img_rqsts.push_back(std::make_tuple(params.working_dir.second / (lin_grad_save_map_pair[1]), lin_grad_clr_map, lin_grad_map_prntr, (lin_grad_save_map_pair[2])));
                    }

        num_objectives = int(objectives_and_constraints.first.size());
        // Make the problem defintions and intialise the objectives and constraints struct.
        prob_defs.reset(new ProblemDefinitions(real_lowerbounds, real_upperbounds, int_lowerbounds, int_upperbounds, params.min_or_max, num_constraints));
//        objectives_and_constrataints = std::make_pair(std::piecewise_construct, std::make_tuple(num_objectives, std::numeric_limits<double>::max()), std::make_tuple(num_constraints));

        //Zonal optimisation objectives go first,

        
        
    }

//    void
//    pushSaveClasifiedMapSpec(std::tuple<std::string, std::string>  arg1)
//    {
//        classified_img_rqsts.push_back(std::make_pair(params.working_dir.second / std::get<1>(arg1), parseColourMapClassified(params.working_dir.second / std::get<0>(arg1))));
//    }
//
//    void
//    pushSaveLinearGradMapSpec(std::tuple<std::string, std::string>  arg1)
//    {
//        lin_grdnt_img_rqsts.push_back(std::make_pair(params.working_dir.second / std::get<1>(arg1), parseColourMapLinearGradient(params.working_dir.second / std::get<0>(arg1))));
//    }
    
    ~GeonamicaOptimiser()
    {
        //        boost::filesystem::remove_all(worker_dir);

        if (delete_wine_dir_on_exit)
        {
            std::this_thread::sleep_for (std::chrono::seconds(1));
            //Check if symbolic link for wine J: exists.
            boost::filesystem::file_status lnk_status = boost::filesystem::symlink_status(params.wine_drive_path.second);
            if ((boost::filesystem::is_symlink(lnk_status)) && (boost::filesystem::exists(params.wine_drive_path.second)))
            {
                boost::filesystem::remove(params.wine_drive_path.second);
            }
        }

        if (delete_wine_prefix_on_exit)
        {
            std::this_thread::sleep_for (std::chrono::seconds(1));
            if (!(boost::filesystem::exists(params.wine_prefix_path.second)))
            {
                boost::filesystem::remove_all(params.wine_prefix_path.second);
            }
        }

        if (boost::filesystem::exists(params.working_dir.second))
        {
            std::this_thread::sleep_for (std::chrono::seconds(1));
            boost::filesystem::remove_all(params.working_dir.second);
        }


    }
    
    
    void
    runGeonamica(std::ofstream & logging_file)
    {
        boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
        if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));
        std::stringstream cmd1, cmd2;
        //Call the model
        if (using_wine) cmd1 << "WINEPREFIX=" << "\"" << params.wine_prefix_path.second.string().c_str() << "\"" << " ";
        if (using_timeout) cmd1 << params.timout_cmd << " ";
        if (using_wine) cmd1 << params.wine_cmd << " ";
        cmd1 << params.geonamica_cmd << " --Reset --Save " << "\"" << wine_working_project << "\"" ;
        if (params.is_logging)
        {
            cmd1 << " >> \"" << logfile.string().c_str() << "\" 2>&1";
            logging_file << "Running: " << cmd1.str() << std::endl;
            logging_file.close();
        }

        int i1 = system(cmd1.str().c_str());
        if (params.is_logging) logging_file.open(logfile.string().c_str(), std::ios_base::app);
        if (!logging_file.is_open()) params.is_logging = false;

        if (using_wine) cmd2 << "WINEPREFIX=" << "\""  << params.wine_prefix_path.second.string().c_str() << "\""  << " ";
        if (using_timeout) cmd2 << params.timout_cmd << " ";
        if (using_wine) cmd2 << params.wine_cmd << " ";
        cmd2 << params.geonamica_cmd << " --Run --Save --LogSettings " << "\"" << wine_working_logging << "\"" << " " << "\"" << wine_working_project << "\"";
        if (params.is_logging)
        {
            cmd2 << " >> \"" << logfile.string().c_str() << "\" 2>&1";
            logging_file << "Running: " << cmd2.str() << std::endl;
            logging_file.close();
        }

        int i2 = system(cmd2.str().c_str());
        if (params.is_logging) logging_file.open(logfile.string().c_str(), std::ios_base::app);
        if (!logging_file.is_open()) params.is_logging = false;
        if (params.is_logging)
        {
            logging_file << "Geonamica run time:\n";
            t->stop();
            t->report();
        }

    }
    
    
    double
    sumMap(blink::raster::gdal_raster<double> & map)
    {
        boost::optional<double> no_data_val = map.noDataVal();
        double sum = 0;
        auto zip = blink::iterator::make_zip_range(std::ref(map));
        if (no_data_val)
        {

            for (auto i : zip)
            {
                const double &val = std::get<0>(i);

                if (val != no_data_val.get())
                {
                    sum += val;
                }
            }
        }
        else
        {
            for (auto i : zip)
            {
                const double &val = std::get<0>(i);
                sum += val;
            }
        }

        return (sum);
    }

    template <typename T> void
    setAllValuesXMLNode(pugi::xml_document & doc, std::string xpath_query, T new_value)
    {
        pugi::xpath_node_set nodes = doc.select_nodes(xpath_query.c_str());
        for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            pugi::xpath_node node = *it;
            node.node().first_child().set_value(std::to_string(new_value).c_str());
        }
    }

    std::vector<double>
    calcObjectives(std::ofstream & logging_file)
    {
        boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
        if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));
        // For each map, sum the metric.
        int metric_num = 0;
        std::vector<double> obj_vals(num_objectives-1, 0);
        BOOST_FOREACH(boost::filesystem::path & map_path, obj_map_paths_minisation)
        {
            obj_vals[metric_num] = 0;
            if (!(boost::filesystem::exists(map_path)))
            {
                for (int year = params.year_start; year <= params.year_end; ++year)
                {
                    boost::filesystem::path map_path_year = map_path.parent_path() / (map_path.filename().string() +  "_" + std::to_string(year) + "-Jan-01 00_00_00.rst");

                    if(boost::filesystem::exists(map_path_year))
                    {
                        blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(map_path_year, GA_ReadOnly);
                        int years_since_start = year - params.year_start;
                        double obj = sumMap(map);
                        obj = obj / std::pow((1+params.discount_rate), years_since_start);
                        obj_vals[metric_num] += obj;
                    }
                }
            }
            else
            {
                blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(map_path, GA_ReadOnly);
                obj_vals[metric_num] = sumMap(map);
            }
            ++metric_num;
        }

        BOOST_FOREACH(boost::filesystem::path & map_path, obj_map_paths_maximisation)
                    {
                        obj_vals[metric_num] = 0;
                        if (!(boost::filesystem::exists(map_path)))
                        {

                            for (int year = params.year_start; year <= params.year_end; ++year)
                            {
                                boost::filesystem::path map_path_year = map_path.parent_path() / (map_path.filename().string() +  "_" + std::to_string(year) + "-Jan-01 00_00_00.rst");

                                if(boost::filesystem::exists(map_path_year))
                                {
                                    blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(map_path_year, GA_ReadOnly);
                                    int years_since_start = year - params.year_start;
                                    double obj = sumMap(map);
                                    obj = obj / std::pow((1+params.discount_rate), years_since_start);
                                    obj_vals[metric_num] += obj;
                                }
                            }
                        }
                        else
                        {
                            blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(map_path, GA_ReadOnly);
                            obj_vals[metric_num] = sumMap(map);
                        }
                        ++metric_num;
                    }

        if (params.is_logging)
        {
            logging_file << "Calculating objectives (i.e. aggregation external to Geonamica) run time:\n";
            t->stop();
            t->report();
        }
        return obj_vals;
    }
    
    
    void
    calculate(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
              boost::filesystem::path save_path = "no_path", boost::filesystem::path _logfile = "unspecified")
    {


        boost::filesystem::path initial_path = boost::filesystem::current_path();
        boost::filesystem::current_path(params.working_dir.second);

        bool do_save = false;
        if (save_path != "no_path") do_save = true;

        std::ofstream logging_file;
        if (params.is_logging)
        {
            if (_logfile == "unspecified")
            {
                std::string filename = "logWorker" + std::to_string(params.evaluator_id)
                                       + "_EvalNo" + std::to_string(eval_count) + "_"
                                       +
                                       boost::posix_time::to_simple_string(
                                               boost::posix_time::second_clock::local_time()) +
                                       ".log";
                this->logfile = params.save_dir.second / filename;
            }
            else
            {
                this->logfile = _logfile;
            }

            if (params.is_logging)
            {
                logging_file.open(this->logfile.string().c_str(), std::ios_base::app);
                if (!logging_file.is_open())
                {
                    params.is_logging = false;
                    std::cout << "attempt to log failed\n";
                }
            }
        }
        boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
        if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));
        
        std::vector<double> & objectives = objectives_and_constraints.first;
        BOOST_FOREACH(double & obj_val, objectives)
        {
            obj_val = 0;
        }


        //reset values of objectives.
        for (int l = 0; l < obj_map_paths_minisation.size(); ++l)
        {
//            objectives[l] = std::numeric_limits<double>::max();
            objectives[l] = 0; // Need to set to zero as summation from 0 across replicates.
        }
        for (int m = 0; m < obj_map_paths_maximisation.size(); ++m)
        {
//            objectives[m] = std::numeric_limits<double>::min();
            objectives[m] = 0; // Need to set to zero as summation from 0 across replicates.
        }


        // Make Zonal map
        {
            objectives.back() = 0.0;
            namespace raster = blink::raster;
            namespace raster_it = blink::iterator;
            raster::gdal_raster<int> zonal_map = raster::open_gdal_raster<int>(this->zonal_map_path, GA_Update);
            auto zip = blink::iterator::make_zip_range(std::ref(zones_delineation_map), std::ref(zonal_map));
            for (auto&& i : zip)
            {
                const int zone = std::get<0>(i);
                if (zones_delineation_no_data_val)
                {
                    if (zone != zones_delineation_no_data_val.get())
                    {
                        int & zone_policy = zone_policies_vec[int_decision_vars[(zone-1)] ]; // zone map index starts at 1; while c++ vectors index starts at 0.
                        std::get<1>(i) = zone_policy;
                        if (not(zone_policy == 0 or zone_policy == 2))
                        {
                            objectives.back() += 1.0; // Zonal_policy = 0 (dscription: 'Other area') or Zonal_policy = 2 (Development Permitted) not really placing something on people, which is what the last objective is about.
                        }
                    }
                }
                else
                {
                    int & zone_policy = zone_policies_vec[int_decision_vars[(zone-1)] ]; // zone map index starts at 1; while c++ vectors index starts at 0.
                    std::get<1>(i) = zone_policy;
                    if (not(zone_policy == 0 or zone_policy == 2))
                    {
                        objectives.back() += 1.0; // Zonal_policy = 0 (description: 'Other area') or Zonal_policy = 2 (Development Permitted) not really placing something on people, which is what the last objective is about.
                    }
                }
            }
        }

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(working_project.string().c_str());

        // Manipulate geoproject with xpath dvs
        {
            for (int j = 0; j < int_dv_xpaths.size(); ++j)
            {
                setAllValuesXMLNode(doc, int_dv_xpaths[j], int_decision_vars[zones.size() + j]);
            }
            for (int k = 0; k < real_dv_xpaths.size(); ++k)
            {
                setAllValuesXMLNode(doc, real_dv_xpaths[k], int_decision_vars[k]);
            }
        }

        std::vector<std::vector<double> > obj_vals_across_replicates;
        for (int j = 0; j < params.replicates; ++j)
        {
            // If geoproject manipulation needed, do it now, here.
            if (params.is_logging) logging_file << "Replicate " << j << "\n";
            setAllValuesXMLNode(doc, "/GeonamicaSimulation/model/modelBlocks/modelBlock[@library=\"\" and @name=\"MB_Land_use_model\"]/CompositeModelBlock/modelBlocks/modelBlock[@library=\"CAModel.dll\" and @name=\"MB_Total_potential\"]/TotalPotentialBlock/Seed", params.rand_seeds[j]);
            doc.save_file(working_project.string().c_str());

            if (do_save)
            {
                // Save a copy of the geoproject file as the current Cmd line runner mangles the GUI aspects preventing it from being loadable in the HGUI interface of Metronamica
                std::string extnsn = working_project.extension().string();
                std::string filename = working_project.stem().string();
                boost::filesystem::path prerun_bck_geoproj = working_project.parent_path() / (filename + "_prerunbck" + extnsn);
                doc.save_file(prerun_bck_geoproj.string().c_str());
            }

            runGeonamica(logging_file);
            std::vector<double> obj_vals = calcObjectives(logging_file);

            boost::filesystem::path save_replicate_path = save_path / ("replicate_" + std::to_string(j));
            if (do_save)
            {
//                boost::filesystem::path save_replicate_path = save_path / ("replicate_" + std::to_string(j));
                //            if (!boost::filesystem::exists(save_replicate_path)) boost::filesystem::create_directory(save_replicate_path);
                if (boost::filesystem::exists(save_replicate_path)) boost::filesystem::remove_all(save_replicate_path);
                copyDir(params.working_dir.second, save_replicate_path);
                typedef std::tuple<boost::filesystem::path, boost::shared_ptr<ColourMapperClassified>,     boost::shared_ptr<MagickWriterClassified>, std::string >  ClassfdImgRqstTuple;
                typedef std::tuple<boost::filesystem::path, boost::shared_ptr<ColourMapperLinearGradient>, boost::shared_ptr<MagickWriterLinearGradient>, std::string >  LinGradntImgRqstTuple;

                BOOST_FOREACH(ClassfdImgRqstTuple & classified_img_request, classified_img_rqsts)
                            {
                                blink::raster::gdal_raster<int> map = blink::raster::open_gdal_raster<int>(std::get<0>(classified_img_request), GA_ReadOnly);
                                boost::filesystem::path save_path = save_replicate_path / std::get<3>(classified_img_request);
                                std::get<2>(classified_img_request)->render(map, save_path);
                            }
                BOOST_FOREACH(LinGradntImgRqstTuple & lin_grad_img_request, lin_grdnt_img_rqsts)
                            {
                                blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(std::get<0>(lin_grad_img_request), GA_ReadOnly);
                                boost::filesystem::path save_path = save_replicate_path / std::get<3>(lin_grad_img_request);
                                std::get<2>(lin_grad_img_request)->render(map, save_path);
                            }
            }
            
            for (int i =0; i < obj_vals.size(); ++i)
            {
                objectives[i] += obj_vals[i];
                if (params.is_logging) logging_file << "Objective " << i << " = " << obj_vals[i] << "\n";
            }
            obj_vals_across_replicates.push_back(obj_vals);

            if (do_save)
            {
                // print value of each replicate objectives.
                std::ofstream objectives_stream;
                objectives_stream.open((save_replicate_path / "objectives.txt").string().c_str());
                if (objectives_stream.is_open())
                {
                    for (int k = 0; k < obj_vals.size(); ++k)
                    {
                        objectives_stream << "Objective " << k << " = " << obj_vals[k] << "\n";
                    }
                }
            }

        }

        if (params.is_logging) logging_file << "Aggregated objectives across replicates:\n";
        for (int i =0; i < objectives.size() - 1; ++i)  //-1 as last objective is number of cells with policy.
        {
            objectives[i] /= params.replicates;
            if (params.is_logging) logging_file << "Objective " << i << " = " << objectives[i] << "\n";
        }

        if (params.is_logging) logging_file << "Objective " << (objectives.size() - 1) << " = " << objectives.back() << "\n";

        if (do_save)
        {
            // print value of each replicate objectives.
            std::ofstream objectives_stream;
            objectives_stream.open((save_path / "objectives.txt").string().c_str());
            if (objectives_stream.is_open())
            {
                for (int k = 0; k < objectives.size(); ++k)
                {
                    objectives_stream << "Objective " << k << " = " << objectives[k] << "\n";
                }
            }
        }
        
        ++eval_count;
        
        if (params.is_logging) logging_file.close();
        
        boost::filesystem::remove_all(previous_logfile);
        previous_logfile = logfile;
        
        boost::filesystem::current_path(initial_path);
        if (params.is_logging)
        {
            logging_file << "Net time for calculating objectives and constraints across all replicates:\n";
            t->stop();
            t->report();
        }
    }
    
    
    
    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars)
    {
        this->calculate(real_decision_vars, int_decision_vars);
        return (objectives_and_constraints);
    }
    
    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars, boost::filesystem::path save_path)
    {
        boost::filesystem::path logging_file = save_path / "log_calculation.log";
        this->calculate(real_decision_vars, int_decision_vars, save_path, logging_file);
        return (objectives_and_constraints);
        
    }
    
    ProblemDefinitionsSPtr getProblemDefinitions()
    {
        return (prob_defs);
    }


    
};


//TODO:
// 1. manipulate geoproject with seed numbers
// 2. Masks for objective maps to exlcude areas of summing up


#endif /* ZonalPolicyOptimiser_hpp */
