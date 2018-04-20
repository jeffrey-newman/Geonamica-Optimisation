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
createCheckpoints(NSGAIIBase<RNG> & optimiser, GeonamicaPolicyParameters & params)
{
    boost::shared_ptr<SavePopCheckpoint> save_pop;
    if (params.save_freq > 0) save_pop.reset(new SavePopCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<SaveFirstFrontCheckpoint> save_front;
    if (params.save_freq > 0) save_front.reset(new SaveFirstFrontCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<Hypervolume> hvol(new Hypervolume(1, params.save_dir.second, Hypervolume::TERMINATION, params.max_gen_hvol));
//    boost::shared_ptr<MetricLinePlot> hvol_plot(new MetricLinePlot(hvol));
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));

    std::string mail_subj("Hypervolume of front from Geonamica optimiser");
    boost::shared_ptr<MailCheckpoint> mail(new MailCheckpoint(params.save_freq, hvol, mail_subj));
    if (!params.email_addresses_2_send_progress.empty())
    {
        for(std::string address: params.email_addresses_2_send_progress)
        {
            mail->addAddress(address);
        }
    }

    boost::shared_ptr<SignalCheckpoint> signal_handler(new SignalCheckpoint(SIGINT));

//    boost::shared_ptr<PlotFrontVTK> plotfront(new PlotFrontVTK);
    if (params.save_freq > 0) optimiser.add_checkpoint(save_pop);
    if (params.save_freq > 0) optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
    if (!params.email_addresses_2_send_progress.empty())
    {
        optimiser.add_checkpoint(mail);
    }

    optimiser.add_checkpoint(maxgen);
    optimiser.add_checkpoint(signal_handler);
}

template void
createCheckpoints(NSGAIIBase<std::mt19937> & optimiser, GeonamicaPolicyParameters & params);