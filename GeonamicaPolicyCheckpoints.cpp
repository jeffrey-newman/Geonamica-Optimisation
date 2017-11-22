//
// Created by a1091793 on 18/8/17.
//

#include "GeonamicaPolicyCheckpoints.hpp"
#include "Checkpoints/SavePopCheckpoint.hpp"
#include "Checkpoints/SaveFirstFrontCheckpoint.hpp"
#include "Metrics/Hypervolume.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/MailCheckpoint.hpp"
#include "Checkpoints/SignalCheckpoint.hpp"


template<typename RNG>
void
createCheckpoints(NSGAII<RNG> & optimiser, GeonamicaPolicyParameters & params)
{
    boost::shared_ptr<SavePopCheckpoint> save_pop(new SavePopCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<SaveFirstFrontCheckpoint> save_front(new SaveFirstFrontCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<Hypervolume> hvol(new Hypervolume(1, params.save_dir.second, Hypervolume::TERMINATION, params.max_gen_hvol));
//    boost::shared_ptr<MetricLinePlot> hvol_plot(new MetricLinePlot(hvol));
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));
    std::string mail_subj("Hypervolume of front from Zonal calibrator ");
    boost::shared_ptr<MailCheckpoint> mail(new MailCheckpoint(10, hvol, mail_subj));
    std::string jeffs_address("jeffrey.newman@adelaide.edu.au");
    mail->addAddress(jeffs_address);
    boost::shared_ptr<SignalCheckpoint> signal_handler(new SignalCheckpoint(SIGINT));

//    boost::shared_ptr<PlotFrontVTK> plotfront(new PlotFrontVTK);
    optimiser.add_checkpoint(save_pop);
    optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
    optimiser.add_checkpoint(mail);
//    optimiser.add_checkpoint(hvol_plot);
//    optimiser.add_checkpoint(plotfront);
    optimiser.add_checkpoint(maxgen);
    optimiser.add_checkpoint(signal_handler);
}

template void
createCheckpoints(NSGAII<std::mt19937> & optimiser, GeonamicaPolicyParameters & params);