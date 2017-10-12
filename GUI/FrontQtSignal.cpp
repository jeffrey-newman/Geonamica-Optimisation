//
// Created by a1091793 on 24/6/17.
//

#include "FrontQtSignal.h"

FrontQtSignal::FrontQtSignal():
num_gens(0)
{

}

bool FrontQtSignal::operator()(PopulationSPtr pop)
{
    ++num_gens;
    FrontsSPtr fronts = pop->getFronts();
    QVector<std::pair<double, double> > new_front;
    std::for_each(fronts->begin()->begin(), fronts->begin()->end(),
                  [&new_front](IndividualSPtr ind)
                    {new_front.append(std::make_pair(ind->getObjective(0), ind->getObjective(1)));}
                 );
    emit nextFront(num_gens, new_front);
    return true;
}