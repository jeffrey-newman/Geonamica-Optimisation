//
//  ZonalPolicyOptimiser.hpp
//  ZonalOptimiser
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
#include "Evaluation.hpp"
#include <blink/raster/PngPlotter.h>
#include <blink/raster/utility.h>
#include <blink/raster/utility.h> // To open rasters
#include <blink/iterator/zip_range.h>
#include <unordered_set>

#include <stdio.h>

#include "ZonalPolicyUtility.hpp"

class ZonalOptimiser : public ObjectivesAndConstraintsBase
{
private:
    
    ZonalPolicyParameters params;
    
    
    boost::filesystem::path working_project;
    std::string wine_working_project;
    
    boost::filesystem::path working_logging;
    std::string wine_working_logging;
    
    boost::filesystem::path zonal_map_path;
    
    std::vector<boost::filesystem::path> obj_map_paths;
    
    boost::filesystem::path zones_delineation_map_path;
    blink::raster::gdal_raster<int> zones_delineation_map;
    
    boost::filesystem::path logfile;
    boost::filesystem::path previous_logfile;
    
    
    int analysisNum;
    int eval_count;
    
    int num_objectives;
    int num_real_decision_vars = 0;
    int num_int_decision_vars;
    int num_constraints = 0;
    
    int min_dv_values  = 0; //lower bounds
    int max_dv_values = 3; // upper bound of x
    // Decision variable mapping:
    // 0 -- 0 Strictly restrict
    // 1 -- 0.5 Weakly restrict
    // 2 -- 1 Allow
    // 3 -- 1.5 Stimulate
    
    ProblemDefinitionsSPtr prob_defs;
    std::pair<std::vector<double>, std::vector<double> > objectives_and_constrataints;
    
    //Copies entire directory - so that each geoproject is running in a different directory.
    bool copyDir(
                 boost::filesystem::path const & source,
                 boost::filesystem::path const & destination
                 )
    {
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(
               !fs::exists(source) ||
               !fs::is_directory(source)
               )
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
            if(!fs::create_directory(destination))
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
    
    
public:
    ZonalOptimiser( ZonalPolicyParameters & _params)
    :
    params(_params),
    eval_count(0),
    num_objectives(params.rel_path_obj_maps.size())
    {
        // Copy project directory into working directory
        std::string temp_dir_template = "Metro_Cal_OF_worker" + std::to_string(params.evaluator_id) + "_%%%%-%%%%";
        params.working_dir.second = boost::filesystem::unique_path(params.working_dir.second / temp_dir_template);
        copyDir(params.template_project_dir.second, params.working_dir.second);
        
        // get paths of important files in working directory.
        working_project = params.working_dir.second / params.rel_path_geoproj;
        wine_working_project =  params.wine_working_dir + "\\\\" + params.rel_path_geoproj;
        zonal_map_path = params.working_dir.second / params.rel_path_zonal_map;
        BOOST_FOREACH(std::string & rel_path, params.rel_path_obj_maps)
        {
            obj_map_paths.push_back(params.working_dir.second / rel_path);
        }
        zones_delineation_map_path = params.working_dir.second / params.rel_path_zones_delineation_map;
        working_logging = params.working_dir.second / params.rel_path_log_specification;
        wine_working_logging = params.wine_working_dir + "\\\\" + params.rel_path_log_specification;
        
        // Load maps into memory
        zones_delineation_map = blink::raster::open_gdal_raster<int>(zones_delineation_map_path, GA_ReadOnly);
        
        
        // Calculate number of zones (this will be equal to the number of decision variables)
        std::unordered_set<int> zones;
        auto zip = blink::iterator::make_zip_range(std::ref(zones_delineation_map));
        for (auto i : zip)
        {
            auto & val = std::get<0>(i);
            zones.emplace(val);
        }
        num_int_decision_vars = zones.size();
        
        // Make the problem defintions and intialise the objectives and constraints struct.
        prob_defs.reset(new ProblemDefinitions(num_real_decision_vars, 0.0,  1.0, num_int_decision_vars, min_dv_values, max_dv_values, params.min_or_max, num_constraints));
        objectives_and_constrataints = std::pair<std::vector<double>, std::vector<double> >(std::piecewise_construct, std::make_tuple(num_objectives, std::numeric_limits<double>::max()), std::make_tuple(num_constraints));
        
        
        
    }
    
    ~ZonalOptimiser()
    {
        //        boost::filesystem::remove_all(worker_dir);
    }
    
    
    void
    runGeonamica(std::ofstream & logging_file)
    {
        std::stringstream cmd1, cmd2;
        //Call the model
        cmd1 << params.model_cmd.first << " --Reset --Save " << wine_working_project ;
        if (params.is_logging) cmd1 << " >> \"" << logfile.c_str() << "\" 2>&1";
        if (params.is_logging) logging_file << "Running: " << cmd1.str() << std::endl;
        if (params.is_logging)  logging_file.close();
        int i1 = system(cmd1.str().c_str());
        if (params.is_logging) logging_file.open(logfile.c_str(), std::ios_base::app);
        if (!logging_file.is_open()) params.is_logging = false;
        
        cmd2 << params.model_cmd.first << " --Run --Save --LogSettings " << wine_working_logging << " " << wine_working_project ;
        if (params.is_logging) cmd2 << " >> \"" << logfile.c_str() << "\" 2>&1";
        if (params.is_logging) logging_file << "Running: " << cmd2.str() << std::endl;
        if (params.is_logging) logging_file.close();
        int i2 = system(cmd2.str().c_str());
        if (params.is_logging) logging_file.open(logfile.c_str(), std::ios_base::app);
        if (!logging_file.is_open()) params.is_logging = false;
    }
    
    
    std::vector<double>
    calcObjectives(std::ofstream & logging_file)
    {
        
        // For each map, sum the metric.
        int metric_num = 0;
        std::vector<double> obj_vals(num_objectives, 0);
        BOOST_FOREACH(boost::filesystem::path & map_path, obj_map_paths)
        {
            blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(map_path, GA_ReadOnly);
            double & sum = obj_vals[metric_num];
            auto zip = blink::iterator::make_zip_range(std::ref(zones_delineation_map));
            for (auto i : zip)
            {
                auto & val = std::get<0>(i);
                sum += val;
            }
            ++metric_num;
        }
        return obj_vals;
    }
    
    
    void
    calculate(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars, bool do_save = false, boost::filesystem::path save_path = "no_path")
    {
        std::string filename = "logWorker" + std::to_string(params.evaluator_id) + "_EvalNo" + std::to_string(eval_count) + "_" + boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) + ".log";
        logfile = params.log_dir.second / filename;
        std::ofstream logging_file;
        if (params.is_logging)
        {
            logging_file.open(logfile.c_str(), std::ios_base::app);
            if (!logging_file.is_open())
            {
                params.is_logging = false;
                std::cout << "attempt to log failed\n";
            }
        }
        
        std::vector<double> & objectives = objectives_and_constrataints.first;
        BOOST_FOREACH(double & obj_val, objectives)
        {
            obj_val = 0;
        }
        
        
        for (int j = 0; j < params.replicates; ++j)
        {
            // If geoproject manipulation needed, do it now, here.
            
            runGeonamica(logging_file);
            std::vector<double> obj_vals = calcObjectives(logging_file);
            
            if (do_save)
            {
                boost::filesystem::path save_replicate_path = save_path / ("replicate_" + std::to_string(j));
                //            if (!boost::filesystem::exists(save_replicate_path)) boost::filesystem::create_directory(save_replicate_path);
                if (boost::filesystem::exists(save_replicate_path)) boost::filesystem::remove_all(save_replicate_path);
                copyDir(params.working_dir.second, save_replicate_path);
            }
            
            for (int i =0; i < obj_vals.size(); ++i)
            {
                objectives[i] += obj_vals[i];
            }
        }
        
        for (int i =0; i < objectives.size(); ++i)
        {
            objectives[i] /= params.replicates;
        }
        
        ++eval_count;
        
        if (params.is_logging) logging_file.close();
        
        boost::filesystem::remove_all(previous_logfile);
        previous_logfile = logfile;
        
        
        
    }
    
    
    
    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars)
    {
        this->calculate(real_decision_vars, int_decision_vars);
        return (objectives_and_constrataints);
    }
    
    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars, boost::filesystem::path save_path)
    {
        this->calculate(real_decision_vars, int_decision_vars, true, save_path);
        return (objectives_and_constrataints);
        
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
