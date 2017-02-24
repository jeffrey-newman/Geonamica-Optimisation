//
//  ZonalPolicyUtility.hpp
//  ZonalOptimiser
//
//  Created by a1091793 on 5/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef ZonalPolicyUtility_hpp
#define ZonalPolicyUtility_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include "Pathify.hpp"

#include "Checkpoints/SavePopCheckpoint.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/PlotFronts.hpp"
#include "Metrics/Hypervolume.hpp"
#include "Checkpoints/ResetMutationXoverFlags.hpp"
#include "Checkpoints/MetricLinePlot.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/MailCheckpoint.hpp"
#include "Checkpoints/SaveFirstFrontCheckpoint.hpp"



struct ZonalPolicyParameters
{
    
    CmdLinePaths model_cmd;  // command which runs the model (like "/bin/timeout --kill-after=20m 19m  /bin/wine Z://PATH/geonamica.exe")
    CmdLinePaths template_project_dir; // path to template project
    CmdLinePaths working_dir;
    std::string  wine_working_dir;
    std::string  rel_path_geoproj; // relative path of geoproject file from geoproject directory (head/root directory)
    std::string  rel_path_zonal_map; // relative path of zonal policy map layer that is being optimised relative to (head/root directory)
    std::vector<std::string> rel_path_obj_maps; // relative paths of objectives maps that we are maximising/minimising relative to (head/root directory)
    std::vector<std::string> min_or_max_str;
    std::vector<MinOrMaxType> min_or_max; //vector of whether the objectives in the maps above are minimised or maximised.
    std::string rel_path_log_specification;  // relative path of logging file from geoproject directory (head/root directory)  - should be in wine format.
    std::string rel_path_zones_delineation_map;  // relative path of a map which delineates the project area into regions wherein zonal policiy is optimised.
    CmdLinePaths  log_dir;
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
    
};

std::pair<std::string, std::string> at_option_parser(std::string const&s)
{
    if ('@' == s[0])
        return std::make_pair(std::string("cfg-file"), s.substr(1));
    else
        return std::pair<std::string, std::string>();
}

int
processOptions(int argc, char * argv[], ZonalPolicyParameters & params)
{
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("model-cmd,m", po::value<std::string>(&params.model_cmd.first), "xecutable string that will run the geonamica model --- without command flags/arguments (like \"/bin/timeout --kill-after=20m 19m  /bin/wine Z://PATH/geonamica.exe\")")
    ("template,t", po::value<std::string>(&params.template_prcoject_dir.first), "path to template geoproject directory")
    ("working-dir,d", po::value<std::string>(&params.working_dir.first)->default_value(boost::filesystem::current_path().string()), "path of directory for storing temp files during running")
    ("wine-work-dir,w", po::value<std::string>(&params.wine_working_dir), "path to working directory (working-dir,d), but in wine path format - e.g. Z:\\path\\to\\working\\dir")
    ("geoproj-file,g", po::value<std::string>(&params.rel_path_geoproj), "name of geoproject file (without full path), relative to template geoproject directory")
    ("zonal-maps,z", po::value<std::string>(&params.rel_path_zonal_map), "name of zonal map (without full path), relative to template geoproject directory")
    ("obj-maps,o",po::value<std::vector<std::string> >(&params.rel_path_obj_maps)->multitoken(), "relative paths wrt template geoproject directory of objective maps")
    ("min-or-max,n",po::value<std::vector<std::string> >(&params.min_or_max_str)->multitoken(), "relative paths wrt template geoproject directory of objective maps")
    ("zone-delineation,e", po::value<std::string>(&params.rel_path_zones_delineation_map), "name of zonal delineation map (without full path), relative to template geoproject directory")
    ("log-dir,l", po::value<std::string>(&params.log_dir.first), "path of the log settings xml file (full path in wine format)")
    ("is-logging,s", po::value<bool>(&params.is_logging), "TRUE or FALSE whether to log the evaluation")
    ("save-dir,v", po::value<std::string>(&params.save_dir.first)->default_value(boost::filesystem::current_path().string()), "path of the directory for writing results and outputs to")
    ("pop-size,p", po::value<int>(&params.pop_size)->default_value(415), "Population size of the NSGAII")
    ("max-gen-no-hvol-improve,x", po::value<int>(&params.max_gen_hvol)->default_value(50), "maximum generations with no improvement in the hypervolume metric - terminaation condition")
    ("max-gen,y", po::value<int>(&params.max_gen)->default_value(500), "Maximum number of generations - termination condition")
    ("save-freq,q", po::value<int>(&params.save_freq)->default_value(1), "how often to save first front, hypervolume metric and population")
    ("replicates,i", po::value<int>(&params.replicates)->default_value(10), "Number of times to rerun Metronamica to account for stochasticity of model for each objective function evaluation")
    ("reseed,r", po::value<std::string>(&params.restart_pop_file.first)->default_value("no_seed"), "File with saved population as initial seed population for GA")    
    ("cfg-file,c", po::value<std::string>(), "can be specified with '@name', too");
    
    po::variables_map vm;
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
    
    pathify(params.model_cmd); //.second = boost::filesystem::path(metro_exe.first);
    pathify(params.template_project_dir);
    pathify(params.working_dir);
    pathify(params.log_dir);
    pathify(params.save_dir);
    pathify(params.restart_pop_file);
    
    
    // Get min or max objectives.
        BOOST_FOREACH(std::string & str, params.min_or_max_str)
    {
        if (str == "MIN") params.min_or_max.push_back(MINIMISATION);
        if (str == "MAX") params.min_or_max.push_back(MAXIMISATION);
        
    }
    
    return (0);
    
}

template<typename RNG>
void
createCheckpoints(NSGAII<RNG> & optimiser, ZonalPolicyParameters & params)
{
    boost::shared_ptr<SavePopCheckpoint> save_pop(new SavePopCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<SaveFirstFrontCheckpoint> save_front(new SaveFirstFrontCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<Hypervolume> hvol(new Hypervolume(1, params.save_dir.second, Hypervolume::TERMINATION, params.max_gen_hvol));
    boost::shared_ptr<MetricLinePlot> hvol_plot(new MetricLinePlot(hvol));
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));
    std::string mail_subj("Hypervolume of front from Zonal calibrator ");
    boost::shared_ptr<MailCheckpoint> mail(new MailCheckpoint(10, hvol, mail_subj));
    std::string jeffs_address("jeffrey.newman@adelaide.edu.au");
    mail->addAddress(jeffs_address);
    
    boost::shared_ptr<PlotFrontVTK> plotfront(new PlotFrontVTK);
    optimiser.add_checkpoint(save_pop);
    optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
    optimiser.add_checkpoint(mail);
    optimiser.add_checkpoint(hvol_plot);
    optimiser.add_checkpoint(plotfront);
    optimiser.add_checkpoint(maxgen);
}


#include "ZonalPolicyOptimiser.hpp"

void
postProcessResults(ZonalOptimiser & zonal_eval, PopulationSPtr pop, ZonalPolicyParameters & params)
{
    Population & first_front = pop->getFronts()->at(0);
    
    int i = 0;
    BOOST_FOREACH(IndividualSPtr ind, first_front)
    {
        std::vector<double> objectives;
        std::vector<double> constraints;
        boost::filesystem::path save_ind_dir = params.save_dir.second / ("individual_" + std::to_string(i++));
        if (!boost::filesystem::exists(save_ind_dir)) boost::filesystem::create_directory(save_ind_dir);
        std::tie(objectives, constraints) = zonal_eval(ind->getRealDVVector(), ind->getIntDVVector(), save_ind_dir);
        ind->setObjectives(objectives);
        ind->setConstraints(constraints);
        std::cout << *ind << std::endl;
    }
    
    boost::filesystem::path save_file = params.save_dir.second / "final_front.xml";
    std::ofstream ofs(save_file.c_str());
    assert(ofs.good());
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(first_front);
    
    boost::filesystem::path save_file2 = params.save_dir.second /  "final_front.txt";
    std::ofstream ofs2(save_file2.c_str());
    assert(ofs2.good());
    ofs2 << pop;
}


#endif /* ZonalPolicyUtility_hpp */
