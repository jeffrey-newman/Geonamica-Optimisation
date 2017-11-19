/*
//
// Created by a1091793 on 18/8/17.
//
//
//  ZonalPolicyOptimiser.hpp
//  GeonamicaOptimiser
//
//  Created by a1091793 on 4/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//
 */

#include "GeonamicaPolicyOptimiser.hpp"

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
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/bind.hpp>
#include "EvaluatorModules/boost_placeholder/dll/import.hpp" // for import_alias
#include "ColourMapperParsers.h"
#include "Evaluation.hpp"
#include <blink/raster/utility.h> // To open rasters
#include <blink/iterator/zip_range.h>
#include <unordered_set>
#include <pugixml.hpp>
#include "UserHomeDirectory.hpp"

#include <stdio.h>

#include "GeonamicaPolicyParameters.hpp"


BOOST_FUSION_ADAPT_STRUCT(
        XPathDV::Point,
        (double, x)
        (double, y)
)

struct XPathDVParser : boost::spirit::qi::grammar<std::string::iterator, boost::spirit::qi::space_type>
{


    XPathDVParser(XPathDV& _dv) : XPathDVParser::base_type(start), dv(_dv)
    {
        namespace qi = boost::spirit::qi;
        namespace ph = boost::phoenix;

        string_parser_quote_delimited = qi::lexeme[qi::lit("\"") >> +(qi::char_ - "\"") >> qi::lit("\"")];   //[_val = _1]

        int_bounds_parser = qi::lit("BOUNDS") >> qi::lit("(")
                >> qi::int_[ph::ref(this->dv.i_lower_bounds) = qi::_1]
                >>  qi::lit(",") >> qi::int_[ph::ref(this->dv.i_upper_bounds) = qi::_1]
                >>  qi::lit(")");

        real_bounds_parser = qi::lit("BOUNDS") >> qi::lit("(")
                >> qi::double_[ph::ref(this->dv.d_lower_bounds) = qi::_1]
                >>  qi::lit(",") >> qi::double_[ph::ref(this->dv.d_upper_bounds) = qi::_1]
                >>  qi::lit(")");

        xpath_parser = string_parser_quote_delimited[ph::ref(this->dv.xpath_2_node) = qi::_1]
                >>  -(qi::lit(":")
                        >> string_parser_quote_delimited[ph::ref(this->dv.attribute_name) = qi::_1]);

        int_dv_parser = (
                qi::lit("INTEGER_ORDERED")[ph::ref(this->dv.dv_type) = XPathDV::INTEGER_ORDERED]
                | qi::lit("INTEGER_UNORDERED")[ph::ref(this->dv.dv_type) = XPathDV::INTEGER_UNORDERED]
        )
                >> qi::lit(":") >> int_bounds_parser
                >>  qi::lit(":") >> xpath_parser;

        real_dv_parser = qi::lit("REAL")[ph::ref(this->dv.dv_type) = XPathDV::REAL]
                >> qi::lit(":") >> real_bounds_parser
                >>  qi::lit(":") >> xpath_parser;

        point_parser = qi::double_ >> qi::lit(",") >> qi::double_;
        spline_parser  = qi::lit("BASE") >> qi::lit("(") >> *(point_parser >> qi::lit(";")) >> point_parser;

        spline_dv_parser = qi::lit("SPLINE")[ph::ref(this->dv.dv_type) = XPathDV::REAL]
                >> qi::lit(":") >> qi::lit("PROPORTIONAL_CHANGE")
                >> qi::lit(":") >> spline_parser[ph::ref(this->dv.base_spline) = qi::_1] >> qi::lit(")")
                >>  qi::lit(":") >> real_bounds_parser
                >>  qi::lit(":") >> xpath_parser;

        start = int_dv_parser | real_dv_parser | spline_dv_parser;

    }

    XPathDV& dv;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> string_parser_quote_delimited;
    boost::spirit::qi::rule<std::string::iterator, XPathDV::Point() > point_parser;
    boost::spirit::qi::rule<std::string::iterator, std::vector<XPathDV::Point>() > spline_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> int_bounds_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> real_bounds_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> xpath_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> int_dv_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> real_dv_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> spline_dv_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> start;
};

struct MapObjParser : boost::spirit::qi::grammar<std::string::iterator, boost::spirit::qi::space_type>
{


    MapObjParser(MapObj& _obj) : MapObjParser::base_type(start), obj(_obj)
    {
        namespace qi = boost::spirit::qi;
        namespace ph = boost::phoenix;

        string_parser_quote_delimited = qi::lexeme[qi::lit("\"") >> +(qi::char_ - "\"") >> qi::lit("\"")];
        years_parser  = qi::lit("YEARS") >> qi::lit("(") >> *((qi::int_ >> qi::lit(";"))[ph::push_back(ph::ref(this->obj.years), qi::_1)]) >> qi::int_[ph::push_back(ph::ref(this->obj.years), qi::_1)] >> qi::lit(")");
        discounting_parser = qi::lit("DISCOUNTING") >> qi::lit("(") >> qi::lit("RATE") >> qi::lit("=") >> qi::double_[ph::ref(this->obj.discount_rate) = qi::_1]
                                                    >> qi::lit(";")
                                                    >> qi::lit("YEAR_PRESENT_VALUE") >> qi::lit("=") >> qi::int_[ph::ref(this->obj.year_present_val) = qi::_1] >> qi::lit(")");
        maximise_parser = qi::no_case[( qi::lit("MAXIMISE") | qi::lit("MAXIMIZE") | qi::lit("MAX"))[ph::ref(this->obj.type) = MapObj::MAXIMISATION] ];
        minimise_parser = qi::no_case[( qi::lit("MINIMISE") | qi::lit("MINIMIZE") | qi::lit("MIN"))[ph::ref(this->obj.type) = MapObj::MINIMISATION] ];

        start = (maximise_parser | minimise_parser )
                >> qi::lit(":") >> string_parser_quote_delimited[ph::ref(this->obj.file_path.first) = qi::_1]
                >> qi::lit(":") >> years_parser
                >> qi::lit(":") >> discounting_parser;

    }

    MapObj& obj;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> string_parser_quote_delimited;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type > years_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> discounting_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> maximise_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> minimise_parser;

    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> start;
};

struct SaveMapParser : boost::spirit::qi::grammar<std::string::iterator, boost::spirit::qi::space_type>
{


    SaveMapParser(SaveMapDetails& _save_map) : MapObjParser::base_type(start), save_map(_save_map)
    {
        namespace qi = boost::spirit::qi;
        namespace ph = boost::phoenix;


        string_parser_quote_delimited = qi::lexeme[qi::lit("\"") >> +(qi::char_ - "\"") >> qi::lit("\"")];
        legend_parser = qi::lit("LEGEND") >> qi::lit("=") >> string_parser_quote_delimited;
        source_raster_parser = qi::lit("PATH") >> qi::lit("=") >> string_parser_quote_delimited;
        years_parser  = qi::lit("YEARS(") >> *(qi::int_[ph::push_back(ph::ref(this->save_map.years), qi::_1)]
                >> qi::lit(";")) >> qi::int_[ph::push_back(ph::ref(this->save_map.years), qi::_1)] >> qi::lit(")");
        diff_raster_parser = (qi::lit("DIFF") >> qi::lit("=") >> string_parser_quote_delimited) | qi::attr(std::string("no_diff"));
        save_path_parser = qi::lit("SAVE_AS") >> qi::lit("=")  >> string_parser_quote_delimited;

        categorised_parser = qi::no_case[( qi::lit("CATEGORISED") | qi::lit("CAT"))[ph::ref(this->save_map.type) = SaveMapDetails::CATEGORISED] ];
        linear_grad_parser = qi::no_case[( qi::lit("LINEAR_GRADIENT") | qi::lit("LIN_GRAD") )[ph::ref(this->save_map.type) = SaveMapDetails::LINEAR_GRADIENT] ];


        start = (categorised_parser | linear_grad_parser )
                >> qi::lit(":") >> legend_parser[ph::ref(this->save_map.legend_file.first) = qi::_1]
                >> qi::lit(":") >> source_raster_parser[ph::ref(this->save_map.source_raster.first) = qi::_1]
                >> qi::lit(":") >> years_parser
                >> qi::lit(":") >> diff_raster_parser[ph::ref(this->save_map.diff_raster.first) = qi::_1]
                >> qi::lit(":") >> save_path_parser[ph::ref(this->save_map.save_path.first) = qi::_1];

    }

    SaveMapDetails& save_map;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> string_parser_quote_delimited;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type > years_parser;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> legend_parser;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> source_raster_parser;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> diff_raster_parser;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> save_path_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> categorised_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> linear_grad_parser;

    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> start;
};


    //Copies entire directory - so that each geoproject is running in a different directory.
    bool GeonamicaOptimiser::copyDir(   boost::filesystem::path const & source,
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
    bool
    GeonamicaOptimiser::copyFilesInDir(
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


template <typename T>
void saveMap(blink::raster::gdal_raster<T> & map, boost::filesystem::path save_path, SaveMapDetails & save_details)
{
    if (save_details.diff_raster.first != "no_diff")
    {
        boost::optional<T> map_no_data = map.noDataVal();
        blink::raster::gdal_raster<T> diff = blink::raster::open_gdal_raster<T>(save_details.diff_raster.second, GA_ReadOnly);
        boost::optional<T> diff_no_data = diff.noDataVal();
        blink::raster::gdal_raster<T> out = blink::raster::create_temp_gdal_raster_from_model<T>(diff);
        auto zip = blink::iterator::make_zip_range(std::ref(map), std::ref(diff), std::ref(out));
        if (map_no_data || diff_no_data)
        {
            for (auto &&i : zip)
            {

                const T map_val = std::get<0>(i);
                const T diff_val = std::get<1>(i);
                if(map_val == map_no_data.value() || diff_val == diff_no_data.value())
                {
                    std::get<2>(i) = map_no_data.value();
                }
                else if(map_val != diff_val)
                {
                    std::get<2>(i) = map_val;
                }
                else
                {
                    std::get<2>(i) = map_no_data.value();
                }
            }
        } else
        {
            for (auto &&i : zip)
            {

                const T map_val = std::get<0>(i);
                const T diff_val = std::get<1>(i);
                if(map_val != diff_val)
                {
                    std::get<2>(i) = map_val;
                }
                else
                {
                    std::get<2>(i) = map_no_data.value();
                }
            }
        }
        if (save_details.type == SaveMapDetails::CATEGORISED)
        {
            save_details.classified_writer->render(map, save_path);
        }
        if (save_details.type == SaveMapDetails::LINEAR_GRADIENT)
        {
            save_details.gradient_writer->render(map, save_path);
        }
    }
    else
    {
        if (save_details.type == SaveMapDetails::CATEGORISED)
        {
            save_details.classified_writer->render(map, save_path);
        }
        if (save_details.type == SaveMapDetails::LINEAR_GRADIENT)
        {
            save_details.gradient_writer->render(map, save_path);
        }

    }
}


    GeonamicaOptimiser::GeonamicaOptimiser( ZonalPolicyParameters & _params)
            :
            params(_params),
            eval_count(0),
            num_objectives(0)
    {

        is_initialised = true;
        // Set up wine prefixes and wine paths.
        using_wine = false;
        if (params.wine_cmd != "no_wine" || params.wine_cmd.empty())
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
            }
            else if (params.wine_prefix_path.first.substr(0, 2) == "na")
            {
                // Do nothing, no prefix path set.
            }
            else
            {
                params.wine_prefix_path.second = params.wine_prefix_path.first;
                if (!(boost::filesystem::exists(params.wine_prefix_path.second)))
                {
                    std::stringstream msg;
                    msg << "Could not find wine prefix on system: " << params.wine_prefix_path.first;
                    initialisation_error_msgs += msg.str() + "; ";
                    std::cout << msg.str() <<  std::endl;
                    is_initialised = false;
                }

//                pathify_mk(params.wine_prefix_path); //although should really already exist....
//                is_initialised = false;
            }

            // Check dosdevices path exists.

            params.wine_drive_path.second = params.wine_prefix_path.second / "dosdevices";
            params.wine_drive_path.first = params.wine_drive_path.second.string();
            if (!(boost::filesystem::exists(params.wine_drive_path.second)))
            {
                std::stringstream msg;
                msg << "Could not find dosdevices in " << params.wine_prefix_path.second
                    << " Is wine installed?";
                initialisation_error_msgs += msg.str() + "; ";
                std::cout << msg.str() <<  std::endl;
                is_initialised = false;
            }

            // Copy project directory into working directory
            std::string temp_dir_template = "Metro_Cal_OF_worker" + std::to_string(params.evaluator_id) + "_%%%%-%%%%";
            params.working_dir.second = boost::filesystem::unique_path(params.working_dir.second / temp_dir_template);
            params.working_dir.first = params.working_dir.second.string();
//            temp_dir_template = params.working_dir.second.filename().string();
            copyDir(params.template_project_dir.second, params.working_dir.second);
//            params.wine_working_dir = params.wine_working_dir + "\\" + temp_dir_template;


            // Create new dosdevice drive to working geoproject file directory.

            if (params.wine_geoproject_disk_drive == "choose_for_me" || params.wine_geoproject_disk_drive.empty())
            {
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
                                    try
                                    {
                                        boost::filesystem::create_directory_symlink(params.working_dir.second, symlinkpath_ext);
                                        //                                params.wine_drive_letter = drive_option;
                                    }
                                    catch (boost::filesystem::filesystem_error & ex)
                                    {
                                        std::stringstream msg;
                                        msg << "Creating symlink from " << params.working_dir.second
                                            << " to " << symlinkpath_ext << " failed";
                                        initialisation_error_msgs += msg.str() + "; ";
                                        std::cout << msg.str() <<  std::endl;
                                        is_initialised = false;
                                    }

                                    params.wine_working_dir = drive_option;
                                    params.wine_drive_path.second = params.wine_drive_path.second / drive_option;
                                    delete_wine_dir_on_exit = true;
                                    break;
                                }
                                if (drive_option == "b:")
                                {
                                    std::stringstream msg;
                                    msg << "Could not make a symlink to the working drive for winedrive";
                                    initialisation_error_msgs += msg.str() + "; ";
                                    std::cout << msg.str() <<  std::endl;
                                    is_initialised = false;
                                }

                            }
            }
            else
            {
                boost::filesystem::path symlinkpath_ext = params.wine_drive_path.second / params.wine_geoproject_disk_drive;
                if (boost::filesystem::exists(symlinkpath_ext))  //exists does not seem to pick up sym links.
                {
                    boost::filesystem::remove(symlinkpath_ext);
                }
                if (boost::filesystem::is_symlink(symlinkpath_ext)) // so we need to use this one to test if something already exists
                {
                    boost::filesystem::remove(symlinkpath_ext);
                }
                try
                {
                    boost::filesystem::create_directory_symlink(params.working_dir.second, symlinkpath_ext);
                    //                                params.wine_drive_letter = drive_option;
                }
                catch (boost::filesystem::filesystem_error & ex)
                {
                    std::stringstream msg;
                    msg << "Creating symlink from " << params.working_dir.second
                        << " to " << symlinkpath_ext << " failed";
                    initialisation_error_msgs += msg.str() + "; ";
                    std::cout << msg.str() <<  std::endl;
                    is_initialised = false;
                }
                params.wine_working_dir = params.wine_geoproject_disk_drive;
                params.wine_drive_path.second = params.wine_drive_path.second / params.wine_geoproject_disk_drive;
                delete_wine_dir_on_exit = true;
            }


        }

        using_timeout = false;
        if (params.timout_cmd != "no_timeout" || params.timout_cmd.empty())
        {
            using_timeout = true;
        }

        // get paths of important files in working directory.
        working_project = params.working_dir.second / params.rel_path_geoproj;
        wine_working_project = params.wine_working_dir + "\\" + params.rel_path_geoproj;




        //Object to hold objectives and constraints.
        objectives_and_constraints = std::pair<std::vector<double>, std::vector<double> >(std::piecewise_construct, std::make_tuple(0), std::make_tuple(num_constraints));

        // Parse list of objectives derived through aggregating a set of maps output from Geonamica
        this->map_objectives.resize(params.rel_path_obj_maps.size());
        for (int l = 0; l < params.rel_path_obj_maps.size(); ++l)
        {
            MapObjParser parser(map_objectives[l]);
            boost::spirit::qi::phrase_parse(params.rel_path_obj_maps[l].begin(), params.rel_path_obj_maps[l].end(), parser, boost::spirit::qi::space);
            parser.obj.file_path.second = params.working_dir.second /  parser.obj.file_path.first;
            if (parser.obj.type == MapObj::MINIMISATION)
            {
                params.min_or_max.push_back(MINIMISATION);
                objectives_and_constraints.first.push_back(std::numeric_limits<double>::max());
            }
            else
            {
                params.min_or_max.push_back(MAXIMISATION);
                objectives_and_constraints.first.push_back(std::numeric_limits<double>::min());
            }
            num_objectives++;
        }

        std::vector<std::string> module_paths;
        std::vector<std::string> constructor_strings;
        namespace qi = boost::spirit::qi;
        namespace ph = boost::phoenix;
        qi::rule<std::string::iterator, std::string()> string_parser_quote_delimited = qi::lit("\"") >> +(qi::char_ - "\"") >> qi::lit("\"");   //[_val = _1]
        qi::rule<std::string::iterator> obj_module_parser = (string_parser_quote_delimited[ph::push_back(ph::ref(module_paths), qi::_1)]
                >>  qi::lit(":")
                >> string_parser_quote_delimited[ph::push_back(ph::ref(constructor_strings), qi::_1)]);

//        qi::debug(obj_module_parser);
        BOOST_FOREACH(std::string & module_info, params.objectives_plugins)
                    {
                        boost::spirit::qi::parse(module_info.begin(), module_info.end(), obj_module_parser);
                    }
        for (int j = 0; j < module_paths.size(); ++j)
        {
            boost::filesystem::path module_path(module_paths[j]);
            boost::shared_ptr<evalModuleAPI> eval_module;
            eval_module = boost::dll::import<evalModuleAPI>(module_path, "eval_module");
            eval_module->configure(constructor_strings[j], params.working_dir.second);
            if (eval_module->isMinOrMax() == MINIMISATION)
            {
                objective_modules.push_back(eval_module);
                params.min_or_max.push_back(MINIMISATION);
                objectives_and_constraints.first.push_back(std::numeric_limits<double>::max());
            }
            else
            {
                objective_modules.push_back(eval_module);
                params.min_or_max.push_back(MAXIMISATION);
                objectives_and_constraints.first.push_back(std::numeric_limits<double>::min());
            }
            num_objectives++;
        }

        //Logging settings
        working_logging = params.working_dir.second / params.rel_path_log_specification_obj;
        wine_working_logging = params.wine_working_dir + "\\" + params.rel_path_log_specification_obj;

        // Zonal optimisation settings
        if (params.rel_path_zones_delineation_map != "no_zonal_dvs" || params.rel_path_zones_delineation_map.empty())
        {
            zonal_map_path = params.working_dir.second / params.rel_path_zonal_map;
            if (params.rel_path_zonal_map == "no_zonal_dvs" || params.rel_path_zonal_map.empty())
            {
                std::stringstream msg;
                msg << "Error: Zonal delineation map specified, but not the zonal map layer in Metronamica";
                initialisation_error_msgs += msg.str() + "; ";
                std::cout << msg.str() <<  std::endl;
                is_initialised = false;
            }

            zones_delineation_map_path = params.working_dir.second / params.rel_path_zones_delineation_map;
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
                        delineations_ids.emplace(val_i);
                    }
                } else
                {
                    delineations_ids.emplace(val_i);
                }
//            if (val != no_data_val)
            }



//            qi::rule<std::string::iterator> zonal_categories_parser = +(qi::int_[ph::push_back(ph::ref(this->zone_categories), qi::_1)]);
            boost::spirit::qi::phrase_parse(params.zonal_map_classes.begin(), params.zonal_map_classes.end(), (+qi::int_)[ph::ref(this->zone_categories) = qi::_1], qi::space);
            int_lowerbounds.resize(delineations_ids.size(), 0);
            int_upperbounds.resize(delineations_ids.size(), this->zone_categories.size() - 1);


            params.min_or_max.push_back(
                    MINIMISATION);  // For minimising the area which has a restrictive (and stimulating?) zonal policies.
            objectives_and_constraints.first.push_back(std::numeric_limits<double>::max());
//            num_objectives++;
        }

        // Parse list of decision variables mapped to the geoproject file through xpaths
        xpath_dvs.resize(params.xpath_dvs.size());
        for (int k = 0; k < params.xpath_dvs.size(); ++k)
        {
            XPathDVParser parser(xpath_dvs[k]);
            boost::spirit::qi::phrase_parse(params.xpath_dvs[k].begin(), params.xpath_dvs[k].end(), parser, qi::space);
            if (xpath_dvs[k].dv_type == XPathDV::REAL)
            {
                real_lowerbounds.push_back(xpath_dvs[k].d_lower_bounds);
                real_upperbounds.push_back(xpath_dvs[k].d_upper_bounds);
            }
            else
            {
                int_lowerbounds.push_back(xpath_dvs[k].i_lower_bounds);
                int_upperbounds.push_back(xpath_dvs[k].i_upper_bounds);
            }
        }


        //Output images of Geonamica output rasters
        this->save_img_rqsts.resize(params.save_maps.size());
        for (int m = 0; m < params.save_maps.size(); ++m)
        {
            SaveMapParser parser(save_img_rqsts[m]);
            boost::spirit::qi::phrase_parse(params.save_maps[m].begin(), params.save_maps[m].end(), parser, boost::spirit::qi::space);
            parser.save_map.legend_file.second = params.working_dir.second /  parser.save_map.legend_file.first;
            parser.save_map.source_raster.second = params.working_dir.second /  parser.save_map.source_raster.first;
            if( ! (parser.save_map.diff_raster.first == "no_diff" or parser.save_map.diff_raster.first.empty()))
            {
                parser.save_map.diff_raster.second = params.working_dir.second / parser.save_map.diff_raster.first;
            }

            if (parser.save_map.type == SaveMapDetails::CATEGORISED)
            {
                parser.save_map.classified_clr_map =  parseColourMapClassified(parser.save_map.legend_file.second);
                parser.save_map.classified_writer.reset(new OpenCVWriterClassified(*parser.save_map.classified_clr_map));
            }
            if (parser.save_map.type == SaveMapDetails::LINEAR_GRADIENT)
            {
                parser.save_map.gradient_clr_map = parseColourMapGradient(parser.save_map.legend_file.second);
                parser.save_map.gradient_writer.reset(new OpenCVWriterGradient(*parser.save_map.gradient_clr_map));
            }
        }


        num_objectives = int(objectives_and_constraints.first.size());
        // Make the problem defintions and intialise the objectives and constraints struct.
        prob_defs.reset(new ProblemDefinitions(real_lowerbounds, real_upperbounds, int_lowerbounds, int_upperbounds, params.min_or_max, num_constraints));
//        objectives_and_constrataints = std::make_pair(std::piecewise_construct, std::make_tuple(num_objectives, std::numeric_limits<double>::max()), std::make_tuple(num_constraints));

        setting_env_vars = false;
        if (!params.windows_env_var.empty() || params.windows_env_var != "unspecified" )
        {
            setting_env_vars = true;
        }

        // Now make batch and running commands for running Geonamica.
        std::stringstream run_command_ss;
        run_bat_file = params.save_dir.second / "run_geonamica.bat";
        run_sh_file = params.save_dir.second / "run_wine.sh";

        std::ofstream bat_file(run_bat_file.string().c_str());
        std::ofstream sh_file(run_sh_file.string().c_str());

        if (setting_env_vars)
        {
            bat_file_contents << "SET " << params.windows_env_var << "\n";
        }
        if (params.with_reset_and_save)
        {
            bat_file_contents << "\"" << params.geonamica_cmd << "\" --Reset --Save " << "\"" << wine_working_project << "\"\n";
        }

        //Call the model to run it.
        if (using_wine && params.wine_prefix_path.first != "na" && params.set_prefix_path)
        {
            sh_file_contents << "WINEPREFIX=" << "\"" << params.wine_prefix_path.second.string().c_str() << "\"" << " ";
        }

        if (using_timeout)
        {
            sh_file_contents << params.timout_cmd << " ";
        }
        if (using_wine)
        {
            sh_file_contents << params.wine_cmd;
        }
        sh_file_contents << " cmd /V /C " << "\"" << run_bat_file.string() << "\"";

        if (params.with_reset_and_save)
        {
            bat_file_contents << "\"" << params.geonamica_cmd << "\" --Run --Save --LogSettings " << "\"" << wine_working_logging << "\""
                 << " " << "\"" << wine_working_project << "\"\n";
        }
        else
        {
            bat_file_contents << "\"" << params.geonamica_cmd << "\" --Run --LogSettings " << "\"" << wine_working_logging << "\""
                 << " " << "\"" << wine_working_project << "\"\n";
        }

        sh_file << sh_file_contents.str();
        bat_file << bat_file_contents.str();
        sh_file.close();
        bat_file.close();
//        run_command = run_command_ss.str();
    }


    GeonamicaOptimiser::~GeonamicaOptimiser()
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
GeonamicaOptimiser::runGeonamica(std::ofstream & logging_file)
{

    boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
    if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));

    std::stringstream run_command_ss;
    run_command_ss << "sh \"" << run_sh_file.string() << "\"";

    if (params.is_logging)
    {
        run_command_ss << " >> \"" << logfile.string().c_str() << "\" 2>&1";
        logging_file << "Running: " << run_command_ss.str() << std::endl;
        logging_file << "With sh file:\n" << sh_file_contents.str() << "\n";
        logging_file << "With BAT file:\n" << bat_file_contents.str() << "\n";
        logging_file.close();
    }

    int i2 = std::system(run_command_ss.str().c_str());

    if (params.is_logging)
    {
        t->stop();
        logging_file.open(logfile.string().c_str(), std::ios_base::app);
    }
    if (!logging_file.is_open()) params.is_logging = false;

    if (params.is_logging)
    {
        logging_file << "Geonamica run time:\n";
        t->report();
    }

}

    double
    GeonamicaOptimiser::sumMap(blink::raster::gdal_raster<double> & map)
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
GeonamicaOptimiser::setXPathDVValue(pugi::xml_document & doc, XPathDV& xpath_details, T new_value)
{


    pugi::xpath_node_set nodes = doc.select_nodes(xpath_details.xpath_2_node.c_str());
    if (nodes.empty())
    {
        std::cout << "Malformed xpath, returns no nodes in geoproject xml\n";
        std::cout << "Xpath given was: " << xpath_details.xpath_2_node << std::endl;
        return;
    }
    for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        if (xpath_details.base_spline.empty())
        {
            pugi::xpath_node node = *it;
            if (node == nullptr)
            {
                std::cout << "Malformed xpath; returns a null node" << std::endl;
            }

            if (xpath_details.attribute_name.empty())
            {
                node.node().set_value(std::to_string(new_value).c_str());
            }
            else
            {
                pugi::xml_attribute attribute = node.node().attribute(xpath_details.attribute_name.c_str());
                if (attribute.empty())
                {
                    std::cout << "Malformed xpath/attribute; returns empty\n";
                    std::cout << "Valid xpath was: " << xpath_details.xpath_2_node << "\n";
                    std::cout << "Invalid attribute name was: " << xpath_details.attribute_name << std::endl;
                }
                attribute.set_value(std::to_string(new_value).c_str());
            }
        }
        else
        {
            pugi::xml_node parentNode = nodes.begin()->parent();
            for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
                pugi::xpath_node node = *it;
                parentNode.remove_child(node.node());
            }

            std::vector<std::pair<double, double> > new_spline;
            for (int j = 0; j < xpath_details.base_spline.size(); ++j)
            {
                new_spline.push_back(std::make_pair(xpath_details.base_spline[j].x, xpath_details.base_spline[j].y * new_value));
            }

            //Recreate the spline node with new points
            pugi::xml_node new_parent = parentNode.append_child("spline");
            typedef std::pair<double, double> Point;
            BOOST_FOREACH(Point p, new_spline)
                        {
                            addPointElement(new_parent, p.first, p.second);
                        }
        }
    }
}


    template <typename T> void
    GeonamicaOptimiser::setAllChildValuesOfXMLNode(pugi::xml_document & doc, std::string  xpath_query, T new_value)
    {
        std::string temp_xpath = xpath_query;
        pugi::xpath_node_set nodes = doc.select_nodes(xpath_query.c_str());
        for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            pugi::xpath_node node = *it;
            node.node().first_child().set_value(std::to_string(new_value).c_str());
        }
    }

    void
    GeonamicaOptimiser::addPointElement(pugi::xml_node & node, double x_val, double & y_val)
    {
        pugi::xml_node elmnt = node.append_child("point");
        elmnt.append_attribute("x").set_value(x_val);
        elmnt.append_attribute("y").set_value(y_val);

    }

//    void
//    GeonamicaOptimiser::setSplineCurveProportional(pugi::xml_document & doc, std::string xpath_query, std::vector<std::vector<double> > & base_spline, double factor)
//    {
//        //Calculate the spline points
//        std::vector<std::pair<double, double> > new_spline;
//        for (int j = 0; j < base_spline.size(); ++j)
//        {
//            new_spline.push_back(std::make_pair(base_spline[j][0], base_spline[j][1] * factor));
//        }
//
//        //Delete the spline node
//        pugi::xpath_node_set nodes = doc.select_nodes(xpath_query.c_str());
//        pugi::xml_node parentNode = nodes.begin()->parent();
//        for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
//            pugi::xpath_node node = *it;
//            parentNode.remove_child(node.node());
//        }
//
//        //Recreate the spline node with new points
//        pugi::xml_node new_parent = parentNode.append_child("spline");
//        typedef std::pair<double, double> Point;
//        BOOST_FOREACH(Point p, new_spline)
//        {
//            addPointElement(new_parent, p.first, p.second);
//        }
//    }

    std::vector<double>
    GeonamicaOptimiser::calcObjectives(std::ofstream & logging_file, const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars)
    {
        boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
        if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));
        // For each map, sum the metric.
        int metric_num = 0;
        std::vector<double> obj_vals(num_objectives, 0);

        for(MapObj & obj: map_objectives)
                    {
                        obj_vals[metric_num] = 0;
                        int num_maps = 0;

                        if (!(boost::filesystem::exists(obj.file_path.second)))
                        {
                            for(int year: obj.years)
                                        {
                                            boost::filesystem::path map_path_year = obj.file_path.second.parent_path() / (obj.file_path.second.stem().string() +  "_" + std::to_string(year) + "-Jan-01 00_00_00" + obj.file_path.second.extension().string());

                                            if(boost::filesystem::exists(map_path_year))
                                            {
                                                ++num_maps;
                                                blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(map_path_year, GA_ReadOnly);
                                                int years_since_start = year - obj.year_present_val;
                                                double obj_val = sumMap(map);
                                                obj_val = obj_val / std::pow((1 + obj.discount_rate), years_since_start);
                                                obj_vals[metric_num] += obj_val;
                                            }
                                        }
                        }
                        else
                        {
                            ++num_maps;
                            blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(obj.file_path.second, GA_ReadOnly);
                            obj_vals[metric_num] = sumMap(map);
                        }
                        if (num_maps == 0) throw std::runtime_error("Unable to find map " + obj.file_path.second.string() + " to aggregate sum");
                        ++metric_num;
                    }

        BOOST_FOREACH(boost::shared_ptr<evalModuleAPI> evaluator, objective_modules)
                    {
                        obj_vals[metric_num] = evaluator->calculate(real_decision_vars, int_decision_vars);
                        ++metric_num;
                    }


        if (params.is_logging)
        {
            logging_file << "Calculating objectives (i.e. aggregation and modules external to Geonamica) run time:\n";
            t->stop();
            t->report();
        }
        return obj_vals;
    }


    void
    GeonamicaOptimiser::calculate(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
              boost::filesystem::path save_path , boost::filesystem::path _logfile )
    {

        if (!is_initialised)
        {
            throw std::runtime_error(initialisation_error_msgs);
            return;
        }

        boost::filesystem::path initial_path = boost::filesystem::current_path();
        boost::filesystem::current_path(params.working_dir.second);

        bool do_save = false;
        if (save_path != "no_path" || !save_path.empty() ) do_save = true;

        // Cycle log files.
        bool delete_previous_logfile = false;
        std::ofstream logging_file;
        if (params.is_logging)
        {
            if (_logfile == "unspecified")
            {
                //Then cycle through log files....
                std::string filename = "logWorker" + std::to_string(params.evaluator_id)
                                       + "_EvalNo" + std::to_string(eval_count) + "_"
                                       +
                                       boost::posix_time::to_simple_string(
                                               boost::posix_time::second_clock::local_time()) +
                                       ".log";
                this->logfile = params.save_dir.second / filename;
                delete_previous_logfile = true;
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
        for(double& obj: objectives)
        {
            obj = 0;
        }

        // Make Zonal map
        if (params.rel_path_zonal_map != "no_zonal_dvs" || params.rel_path_zonal_map == "")
        {
            int min_delineated_id = *delineations_ids.begin();
            int max_delineated_id = *delineations_ids.end();
            std::vector<int> zonal_values(max_delineated_id - min_delineated_id + 1, -1);

            int zone_dv_index = 0;
            for (const int &delineation_id : delineations_ids)
            {
                zonal_values[delineation_id - min_delineated_id] = int_decision_vars[zone_dv_index++];
            }

            objectives.back() = 0.0;
            namespace raster = blink::raster;
            namespace raster_it = blink::iterator;
            raster::gdal_raster<int> zonal_map = raster::open_gdal_raster<int>(this->zonal_map_path, GA_Update);
            auto zip = blink::iterator::make_zip_range(std::ref(zones_delineation_map), std::ref(zonal_map));
            if (zones_delineation_no_data_val)
            {
                for (auto &&i : zip)
                {
                    const int zone = std::get<0>(i);
                    if (zone != zones_delineation_no_data_val.get())
                    {
                        int zone_policy = zonal_values[zone - min_delineated_id];
                        std::get<1>(i) = zone_policy;
                    }
                }
            }


            else
            {
                for (auto &&i : zip)
                {
                    const int zone = std::get<0>(i);
                    int zone_policy = zonal_values[zone - min_delineated_id];
                    std::get<1>(i) = zone_policy;
                }

            }
        }


        // Manipulate geoproject with xpath dvs
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(working_project.string().c_str());
        {
            int k = 0;
            int j = delineations_ids.size();
            for (XPathDV & dv : this->xpath_dvs)
            {
                if (dv.dv_type == XPathDV::REAL) setXPathDVValue(doc, dv, real_decision_vars[k++]);
                else setXPathDVValue(doc, dv, int_decision_vars[j++]);
            }
//            for (int j = 0; j < int_dv_xpaths.size(); ++j)
//            {
//                setAllValuesXMLNode(doc, int_dv_xpaths[j], int_decision_vars[zones.size() + j]);
//            }
//            for (int k = 0; k < real_dv_xpaths.size(); ++k)
//            {
//                setAllValuesXMLNode(doc, real_dv_xpaths[k], real_decision_vars[k]);
//            }
//            int real_dv_temp_index = real_dv_xpaths.size();
//            for (int l = 0; l < spline_proportion_xpaths.size(); ++l)
//            {
//                setSplineCurveProportional(doc, spline_proportion_xpaths[l], spline_curves[l], real_decision_vars[real_dv_temp_index+l]);
//            }
        }

        // Calculate objectives for a number of replicate runs of Geonamica
        std::vector<std::vector<double> > obj_vals_across_replicates;
        for (int j = 0; j < params.replicates; ++j)
        {
            // Set stochastic seed for landuse model part
            if (params.is_logging) logging_file << "Replicate " << j << "\n";
            setAllChildValuesOfXMLNode(doc, "/GeonamicaSimulation/model/modelBlocks/modelBlock[@library=\"\" and @name=\"MB_Land_use_model\"]/CompositeModelBlock/modelBlocks/modelBlock[@library=\"CAModel.dll\" and @name=\"MB_Total_potential\"]/TotalPotentialBlock/Seed", params.rand_seeds[j]);
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
            std::vector<double> obj_vals = calcObjectives(logging_file, real_decision_vars, int_decision_vars);

            // If output of running Geonamica is saved.... (this is not logging which is managed by Geopnbamica iotself, but the generation of images of Geonamica logged output.
            boost::filesystem::path save_replicate_path = save_path / ("replicate_" + std::to_string(j));
            if (do_save) {
//                boost::filesystem::path save_replicate_path = save_path / ("replicate_" + std::to_string(j));
                //            if (!boost::filesystem::exists(save_replicate_path)) boost::filesystem::create_directory(save_replicate_path);
                if (boost::filesystem::exists(save_replicate_path)) boost::filesystem::remove_all(save_replicate_path);
                copyDir(params.working_dir.second, save_replicate_path);

                BOOST_FOREACH(SaveMapDetails &save_details, this->save_img_rqsts) {
                                if (!(boost::filesystem::exists(save_details.source_raster.second))) {
                                    BOOST_FOREACH(int year, save_details.years) {
                                                    boost::filesystem::path map_path_year =
                                                            save_details.source_raster.second.parent_path() /
                                                            (save_details.source_raster.second.filename().string() +
                                                             "_" + std::to_string(year) + "-Jan-01 00_00_00.rst");

                                                    if (boost::filesystem::exists(map_path_year)) {
                                                        boost::filesystem::path save_path = save_replicate_path /
                                                                                            (save_details.save_path.first +
                                                                                             "_" +
                                                                                             std::to_string(year) +
                                                                                             ".png");
                                                        if (save_details.type == SaveMapDetails::CATEGORISED) {
                                                            blink::raster::gdal_raster<int> map = blink::raster::open_gdal_raster<int>(
                                                                    map_path_year, GA_ReadOnly);
                                                            saveMap(map, save_path, save_details);
                                                        }
                                                        if (save_details.type == SaveMapDetails::LINEAR_GRADIENT) {
                                                            blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(
                                                                    map_path_year, GA_ReadOnly);
                                                            saveMap(map, save_path, save_details);
                                                        }


                                                    }
                                                }
                                } else {
                                    boost::filesystem::path save_path =
                                            save_replicate_path / (save_details.save_path.first + ".png");
                                    if (save_details.type == SaveMapDetails::CATEGORISED) {
                                        blink::raster::gdal_raster<int> map = blink::raster::open_gdal_raster<int>(
                                                save_details.source_raster.second, GA_ReadOnly);
                                        saveMap(map, save_path, save_details);
                                    }
                                    if (save_details.type == SaveMapDetails::LINEAR_GRADIENT) {
                                        blink::raster::gdal_raster<double> map = blink::raster::open_gdal_raster<double>(
                                                save_details.source_raster.second, GA_ReadOnly);
                                        saveMap(map, save_path, save_details);
                                    }
                                }
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

        if (delete_previous_logfile) boost::filesystem::remove_all(previous_logfile);
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
    GeonamicaOptimiser::operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars)
    {
        this->calculate(real_decision_vars, int_decision_vars);
        return (objectives_and_constraints);
    }

    std::pair<std::vector<double>, std::vector<double> > &
    GeonamicaOptimiser::operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars, boost::filesystem::path save_path)
    {
        boost::filesystem::path logging_file = save_path / "log_calculation.log";
        this->calculate(real_decision_vars, int_decision_vars, save_path, logging_file);
        return (objectives_and_constraints);

    }

    ProblemDefinitionsSPtr
    GeonamicaOptimiser::getProblemDefinitions()
    {
        return (prob_defs);
    }



//TODO:
// 1. manipulate geoproject with seed numbers
// 2. Masks for objective maps to exlcude areas of summing up




