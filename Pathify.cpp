//
// Created by a1091793 on 19/8/17.
//


#include <iostream>
#include <boost/filesystem.hpp>
#include "Pathify.hpp"


bool
pathify(CmdLinePaths & path)
{
    if (path.first.empty())
    {
        std::cout << "Warning: path given to pathify in Pathify.cpp is empty\n";
        return false;
    }
    path.second = boost::filesystem::path(path.first);
    if (!(boost::filesystem::exists(path.second)))
    {
        std::cout << "Warning: path " << path.first << " does not exist\n";
        return false;
    }
    return true;
}


bool
pathify_mk(CmdLinePaths & path)
{

    if (path.first.empty())
    {
        std::cout << "Warning: path given to pathify in Pathify.cpp is empty\n";
        return false;
    }
        path.second = boost::filesystem::path(path.first);
        if (!(boost::filesystem::exists(path.second)))
        {
            try
            {
                boost::filesystem::create_directories(path.second);
                std::cout << "path " << path.first << " did not exist, so created\n";
            }
            catch(boost::filesystem::filesystem_error& e)
            {
                std::cout << "Attempted to create " << path.first << " but was unable\n";
                std::cout << e.what() << "\n";
                return false;
            }

            return true;
        }
        return true;

}


