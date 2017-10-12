//
// Created by a1091793 on 19/8/17.
//


#include <iostream>
#include <boost/filesystem.hpp>
#include "Pathify.hpp"

void
pathify(CmdLinePaths & path)
{
    path.second = boost::filesystem::path(path.first);
    if (!(boost::filesystem::exists(path.second)))
    {
        std::cout << "Warning: path " << path.first << " does not exist\n";
    }
}

void
pathify_mk(CmdLinePaths & path)
{
    path.second = boost::filesystem::path(path.first);
    if (!(boost::filesystem::exists(path.second)))
    {
        boost::filesystem::create_directories(path.second);
        std::cout << "path " << path.first << " did not exist, so created\n";
    }
}
