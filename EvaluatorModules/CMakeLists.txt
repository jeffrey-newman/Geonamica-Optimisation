SET(ImplementationCostSrcs
        ImplementationCost.hpp
        ImplementationCost.cpp
        ModuleAPI.hpp
        )

add_library(implementcosts SHARED ${ImplementationCostSrcs})
