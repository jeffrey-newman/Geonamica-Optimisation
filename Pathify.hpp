#ifndef PATHIFY_H
#define PATHIFY_H

#include <string>
#include <boost/filesystem.hpp>

typedef std::pair<std::string, boost::filesystem::path> CmdLinePaths;

void
pathify(CmdLinePaths & path);

void
pathify_mk(CmdLinePaths & path);

#endif // PATHIFY_H
