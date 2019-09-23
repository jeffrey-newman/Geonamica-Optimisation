# Geonamica-Optimisation
General  optimisation evaluator for use with RIKS Geonamic projects (an generalised and extensable framework for decision support systems, particularly for environmental, spatial and temporal problems) and my NSGAII-backend.

Objectives: aggregated from any Geonamica-loggable output. Current options are:
1. averaging a map
2. averaging a series of maps with temporal discounting (future values discounted to present)

Decision variables:
1. whether each zone in a zonal layer is actively stimulated, allowed, weakly restricted or completely restricted. This also adds an decision variable relating to how many cells in the zonal layer that are in a state that is not 'allowed' therefore favouring a 'free' market.
2. Any geoproject defined variable (or vector of variables) which is obtainable using a PUGI-compatible xpath statement 

Information on dependencies and compiling is found in the wiki: https://github.com/jeffrey-newman/Geonamica-Optimisation/wiki/Compilation-notes
