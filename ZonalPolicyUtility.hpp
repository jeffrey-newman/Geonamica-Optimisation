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
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include "Pathify.hpp"

#include "NSGAII.hpp"
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
    std::string timout_cmd; //cmd to run everything through another program which kills model on timer, incase it gets stuck/spins/hangs
    std::string wine_cmd; // cmd to run geonamica model through wine, emulation for running on linux or mac.
    std::string geonamica_cmd;  // path to geonamicaCmd.exe. Usually UNIX or Wine path both should work Z://PATH/geonamica.exe")
    CmdLinePaths template_project_dir; // path to template project
    CmdLinePaths working_dir;
    CmdLinePaths wine_prefix_path;
    CmdLinePaths wine_drive_path;
    std::string wine_drive_letter;
    std::string  wine_working_dir;
    std::string  rel_path_geoproj; // relative path of geoproject file from geoproject directory (head/root directory)
    std::string  rel_path_zonal_map; // relative path of zonal policy map layer that is being optimised relative to (head/root directory.)
    std::vector<std::string> rel_path_obj_maps; // relative paths of objectives maps that we are maximising/minimising relative to (head/root directory)
    std::vector<std::string> min_or_max_str;
    std::vector<MinOrMaxType> min_or_max; //vector of whether the objectives in the maps above are minimised or maximised.
    std::string rel_path_log_specification;  // relative path of logging file from geoproject directory (head/root directory)  - should be in wine format.
    std::string rel_path_zones_delineation_map;  // relative path of a map which delineates the project area into regions wherein zonal policiy is optimised.
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
    int year_start;
    int year_end;
    double discount_rate;
    
};

std::string
userHomeDir() {
    std::string path;
    char const *home = getenv("HOME");
    if (home or ((home = getenv("USERPROFILE")))) {
        path.append(home);
    } else {
        char const *hdrive = getenv("HOMEDRIVE"),
                *hpath = getenv("HOMEPATH");
        assert(hdrive);  // or other error handling
        assert(hpath);
        path.append(std::string(hdrive) + hpath);
    }
    return path;
}

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

    boost::filesystem::path deafult_working_dir = boost::filesystem::path(userHomeDir()) / ".geonamicaZonalOpt/working_dir";

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("timeout-cmd,u", po::value<std::string>(&params.timout_cmd)->default_value("no_timeout"), "[optional] excutable string that will run the timeout cmd --- to run everything through another program which kills model on timer, incase it gets stuck/spins/hangs")
    ("wine-cmd,f", po::value<std::string>(&params.wine_cmd)->default_value("no_wine"), "[optional] path to wine (emulator) excutable ")
    ("geonamica-cmd,m", po::value<std::string>(&params.geonamica_cmd), "excutable string that will run the geonamica model --- without command flags/arguments (like Z://PATH/geonamica.exe\")")
    ("template,t", po::value<std::string>(&params.template_project_dir.first), "path to template geoproject directory")
    ("working-dir,d", po::value<std::string>(&params.working_dir.first)->default_value(deafult_working_dir.string()), "path of directory for storing temp files during running")
    ("wine-prefix-path,w", po::value<std::string>(&params.wine_prefix_path.first)->default_value("use_home_path"), "Path to the wine prefix to use. Subfolder should contain dosdevices. To use default in home drive, specify <use_home_drive> to generate new prefix use <generate>")
//            ("wine-drive-path,f", po::value<std::string>(&params.wine_prefix_path.first)->default_value("do not test"), "Path of root directory of wine drive")
//            ("wine-drive-letter,g", po::value<std::string>(&params.wine_drive_letter), "Letter of drive to make symlink for - i.e. C for 'C:' or Z for 'Z:' etc ")
//    ("wine-work-dir,w", po::value<std::string>(&params.wine_working_dir), "path to working directory (working-dir,d), but in wine path format - e.g. Z:\\path\\to\\working\\dir")
    ("geoproj-file,g", po::value<std::string>(&params.rel_path_geoproj), "name of geoproject file (without full path), relative to template geoproject directory. Needs to be in top level at the moment")
    ("zonal-maps,z", po::value<std::string>(&params.rel_path_zonal_map), "name of zonal map (without full path), relative to template geoproject directory. This needs to be GDAL create writable, so NOT ASCII grid format")
    ("obj-maps,o",po::value<std::vector<std::string> >(&params.rel_path_obj_maps)->multitoken(), "relative paths wrt template geoproject directory of objective maps")
    ("min-or-max,n",po::value<std::vector<std::string> >(&params.min_or_max_str)->multitoken(), "whether the aggregated value in the obj-maps are to be minimised (specify MIN) or maximised (specify MAX)")
    ("zone-delineation,e", po::value<std::string>(&params.rel_path_zones_delineation_map), "name of zonal delineation map (without full path), relative to template geoproject directory")
    ("log-spec,l", po::value<std::string>(&params.rel_path_log_specification), "path of the log settings xml file (relative to template geoproject directory in in wine format)")
    ("is-logging,s", po::value<bool>(&params.is_logging), "TRUE or FALSE whether to log the evaluation")
    ("save-dir,v", po::value<std::string>(&params.save_dir.first)->default_value(boost::filesystem::current_path().string()), "path of the directory for writing results and outputs to")
    ("pop-size,p", po::value<int>(&params.pop_size)->default_value(415), "Population size of the NSGAII")
    ("max-gen-no-hvol-improve,x", po::value<int>(&params.max_gen_hvol)->default_value(50), "maximum generations with no improvement in the hypervolume metric - terminaation condition")
    ("max-gen,y", po::value<int>(&params.max_gen)->default_value(500), "Maximum number of generations - termination condition")
    ("save-freq,q", po::value<int>(&params.save_freq)->default_value(1), "how often to save first front, hypervolume metric and population")
    ("replicates,i", po::value<int>(&params.replicates)->default_value(10), "Number of times to rerun Metronamica to account for stochasticity of model for each objective function evaluation")
    ("reseed,r", po::value<std::string>(&params.restart_pop_file.first)->default_value("no_seed"), "File with saved population as initial seed population for GA")    
    ("year-start,a",po::value<int>(&params.year_start),"Start year for objective map logging - only valid if objective logging file stem is only given and not full filename")
    ("year-end,b",po::value<int>(&params.year_end),"End year for objective map logging - only valid if objective logging file stem is only given and not full filename")
    ("discount-rate,d",po::value<double>(&params.discount_rate)->default_value(0.0),"discount rate for objectives (applies to all of them)")
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
    
//    pathify(params.geonamica_cmd); //.second = boost::filesystem::path(metro_exe.first);



        pathify_mk(params.working_dir);



//        boost::filesystem::path symlinkpath_ext = symlinkpath / drive_option;
//        //Check if symbolic link for wine J: exists.
//        boost::filesystem::file_status lnk_status = boost::filesystem::symlink_status(symlinkpath_ext);
//        if (!(boost::filesystem::is_symlink(lnk_status)) || !(boost::filesystem::exists(symlinkpath_ext)))
//        {
//            boost::filesystem::create_directory_symlink(params.working_dir.second, symlinkpath_ext);
//        }
//        system(("ls " + symlinkpath.string()).c_str());


    pathify(params.template_project_dir);

    //pathify(params.log_dir);
    pathify_mk(params.save_dir);

    if (params.restart_pop_file.first != "no_seed")
    {
        pathify(params.restart_pop_file);
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
//    boost::shared_ptr<MetricLinePlot> hvol_plot(new MetricLinePlot(hvol));
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));
    std::string mail_subj("Hypervolume of front from Zonal calibrator ");
    boost::shared_ptr<MailCheckpoint> mail(new MailCheckpoint(10, hvol, mail_subj));
    std::string jeffs_address("jeffrey.newman@adelaide.edu.au");
    mail->addAddress(jeffs_address);
    
//    boost::shared_ptr<PlotFrontVTK> plotfront(new PlotFrontVTK);
    optimiser.add_checkpoint(save_pop);
    optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
//    optimiser.add_checkpoint(mail);
//    optimiser.add_checkpoint(hvol_plot);
//    optimiser.add_checkpoint(plotfront);
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


void
cleanup(ZonalPolicyParameters & params)
{
    if (params.wine_prefix_path.first != "do not test") {

        boost::filesystem::path symlinkpath("~/.wine/dosdevices");
        symlinkpath = symlinkpath / params.wine_drive_letter;
//Check is symbolic link for wine J: exists.
        boost::filesystem::file_status lnk_status = boost::filesystem::status(symlinkpath);
        if ((boost::filesystem::exists(lnk_status)))
        {
            boost::filesystem::remove_all(symlinkpath);
        }
    }

    if (boost::filesystem::exists(params.working_dir.second))
    {
        boost::filesystem::remove_all(params.working_dir.second);
    }

}

#endif /* ZonalPolicyUtility_hpp */
