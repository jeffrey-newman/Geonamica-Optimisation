//
// Created by a1091793 on 18/8/17.
//
//
//#include "GeonamicaPolicyPostProcess.hpp"
//#include <boost/archive/xml_oarchive.hpp>
//#include <boost/serialization/shared_ptr.hpp>
//
//
//void
//postProcessResults(GeonamicaOptimiser & zonal_eval, PopulationSPtr pop, GeonamicaPolicyParameters & params, boost::filesystem::path save_dir, bool process_first_front)
//{
//    Population * pop_2_process;
//    if (save_dir == "")
//    {
//        save_dir = params.save_dir.second;
//    }
//
//    if (process_first_front)
//    {
//        pop_2_process = &(pop->getFronts()->at(0));
//    }
//    else
//    {
//        pop_2_process = pop.get();
//    }
//
//    ObjectiveValueCompator obj_comparator(0);
//    std::sort(pop_2_process->begin(), pop_2_process->end(), obj_comparator);
//
//    int i = 0;
//    BOOST_FOREACH(IndividualSPtr ind, *pop_2_process)
//                {
//                    std::vector<double> objectives;
//                    std::vector<double> constraints;
//                    boost::filesystem::path save_ind_dir = save_dir / ("individual_" + std::to_string(i++));
//                    if (!boost::filesystem::exists(save_ind_dir)) boost::filesystem::create_directories(save_ind_dir);
//                    std::tie(objectives, constraints) = zonal_eval(ind->getRealDVVector(), ind->getIntDVVector(), save_ind_dir);
//                    ind->setObjectives(objectives);
//                    ind->setConstraints(constraints);
//                    std::cout << *ind << std::endl;
//                }
//
//    boost::filesystem::path save_file = save_dir / "final_front.xml";
//    std::ofstream ofs(save_file.c_str());
//    assert(ofs.good());
//    boost::archive::xml_oarchive oa(ofs);
//    oa << boost::serialization::make_nvp("Population", *pop_2_process);
//
//    boost::filesystem::path save_file2 = save_dir /  "final_front.txt";
//    std::ofstream ofs2(save_file2.c_str());
//    assert(ofs2.good());
//    ofs2 << *pop_2_process;
//}



//void
//cleanup(GeonamicaPolicyParameters & params)
//{
//    if (params.wine_prefix_path.first != "do not test") {
//
//        boost::filesystem::path symlinkpath("~/.wine/dosdevices");
//        symlinkpath = symlinkpath / params.wine_drive_letter;
////Check is symbolic link for wine J: exists.
//        boost::filesystem::file_status lnk_status = boost::filesystem::status(symlinkpath);
//        if ((boost::filesystem::exists(lnk_status)))
//        {
//            boost::filesystem::remove_all(symlinkpath);
//        }
//    }
//
//    if (boost::filesystem::exists(params.working_dir.second))
//    {
//        boost::filesystem::remove_all(params.working_dir.second);
//    }
//
//}