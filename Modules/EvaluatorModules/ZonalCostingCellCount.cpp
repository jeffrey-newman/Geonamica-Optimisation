//
// Created by a1091793 on 18/11/17.
//



#include <boost/spirit/include/qi.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "blink/raster/utility.h"
#include "blink/iterator/zip_range.h"
#include "ZonalCostingCellCount.h"


struct ZoneCostingCellCountParser : boost::spirit::qi::grammar<std::string::iterator, boost::spirit::qi::space_type>
{


    ZoneCostingCellCountParser(ZoneCostingParameters& _params) : ZoneCostingCellCountParser::base_type(start), params(_params)
    {
        namespace qi = boost::spirit::qi;
        namespace ph = boost::phoenix;

        string_parser_single_quote_delimited = qi::lexeme[qi::lit("\'") >> +(qi::char_ - "\'") >> qi::lit("\'")];   //[_val = _1]

//        project_path_parser = qi::lit("Project") >> qi::lit(":")
//                                                 >> string_parser_single_quote_delimited[ph::ref(this->params.project_path.first) = qi::_1]
//                                                 >> qi::lit(";");
        zone_raster_path_parser = qi::lit("ZoneRaster") >> qi::lit(":")
                                                 >> string_parser_single_quote_delimited[ph::ref(this->params.zonal_raster.first) = qi::_1]
                                                 >> qi::lit(";");
        exclusion_vals_parser = qi::lit("ExclusionVals") >> qi::lit(":")
                                              >> qi::int_[ph::push_back(ph::ref(this->params.exclusion_vals), qi::_1)]
                                              >>  *(qi::lit(",") >> qi::int_[ph::push_back(ph::ref(this->params.exclusion_vals), qi::_1)]);

        start = zone_raster_path_parser >> exclusion_vals_parser;

    }

    ZoneCostingParameters& params;
    boost::spirit::qi::rule<std::string::iterator, std::string(), boost::spirit::qi::space_type> string_parser_single_quote_delimited;
//    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> project_path_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> zone_raster_path_parser;
    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> exclusion_vals_parser;

    boost::spirit::qi::rule<std::string::iterator, boost::spirit::qi::space_type> start;
};

const std::string ZonalCostingCellCount::name() const
{
    return ("Zonal Costing by counting zonal cells");
}


void ZonalCostingCellCount::configure(const std::string _configure_string, const boost::filesystem::path _geoproj_dir)
{
    //constructor string format: Project:<'path to project'>; ZoneRaster: <'relative path to zone raster'>; ExclusionVals: <valuue1>, <value2>
    std::string config_string_copy = _configure_string;
    ZoneCostingCellCountParser parser(params);
    boost::spirit::qi::phrase_parse(config_string_copy.begin(), config_string_copy.end(), parser, boost::spirit::qi::space);
//    pathify(params.project_path);
    params.zonal_raster.second = _geoproj_dir / params.zonal_raster.first;
}


std::shared_ptr<const std::vector<double> >
ZonalCostingCellCount::calculate(const std::vector<double> &_real_decision_vars,
                                        const std::vector<int> &_int_decision_vars)
{
    int exclusion_cell_count = 0;
    blink::raster::gdal_raster<int> zonal_map = blink::raster::open_gdal_raster<int>(this->params.zonal_raster.second, GA_ReadOnly);
    auto zip = blink::iterator::make_zip_range(std::ref(zonal_map));
    for (const int & exclusion_val : this->params.exclusion_vals)
    {
        for (auto&& i : zip)
        {
            const int zone = std::get<0>(i);
            if (zone == exclusion_val)
            {
                ++exclusion_cell_count;
            }
        }
    }
    std::shared_ptr<std::vector<double> > obj_vals(new std::vector<double>);
    obj_vals->push_back(double(exclusion_cell_count));
    return (obj_vals);
}

const std::vector<MinOrMaxType>&
ZonalCostingCellCount::isMinOrMax() const
{
    return (min_or_max_types);
}


extern "C" BOOST_SYMBOL_EXPORT ZonalCostingCellCount eval_module;
ZonalCostingCellCount eval_module;
