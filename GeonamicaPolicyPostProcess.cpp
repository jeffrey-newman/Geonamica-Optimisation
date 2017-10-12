//
// Created by a1091793 on 18/8/17.
//

#include "GeonamicaPolicyPostProcess.hpp"
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>


void
postProcessResults(GeonamicaOptimiser & zonal_eval, PopulationSPtr pop, ZonalPolicyParameters & params)
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
    ofs2 << first_front;
}

//void
//cleanup(ZonalPolicyParameters & params)
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

//}