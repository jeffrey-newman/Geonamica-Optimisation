SET(ImplementationCostSrcs
        ImplementationCost.hpp
        ImplementationCost.cpp
        ModuleAPI.hpp
        )

add_library(implementcosts SHARED ${ImplementationCostSrcs})

SET(ZonalCostingCellCountSrcs
        ZonalCostingCellCount.cpp
        ZonalCostingCellCount.h
        ModuleAPI.hpp
        )

add_library(zonalcostscellcount SHARED ${ZonalCostingCellCountSrcs})