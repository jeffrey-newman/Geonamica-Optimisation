//
// Created by a1091793 on 19/8/17.
//

#ifndef GEON_OPT_USERHOMEDIRECTORY_HPP
#define GEON_OPT_USERHOMEDIRECTORY_HPP

inline
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

#endif //GEON_OPT_USERHOMEDIRECTORY_HPP
