//
// Created by a1091793 on 12/6/17.
//


#include "GeonamicaOptGUICheckpoints.h"
#include <csignal>
#include "Checkpoints/SavePopCheckpoint.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/PlotFronts.hpp"
#include "Metrics/Hypervolume.hpp"
#include "Checkpoints/ResetMutationXoverFlags.hpp"
#include "Checkpoints/MetricLinePlot.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/MailCheckpoint.hpp"
#include "Checkpoints/SaveFirstFrontCheckpoint.hpp"
#include "Checkpoints/SignalCheckpoint.hpp"
#include "OptimiserWorker.h"
#include "MetricQtSignal.h"
#include "FrontQtSignal.h"
#include "QtDialogueCheckpoint.h"
#include <QObject>


template<typename RNG>
void
createCheckpointsQtGUI(NSGAII<RNG> & optimiser, GeonamicaPolicyParameters & params, OptimiserWorker * thread)
{
    boost::shared_ptr<SavePopCheckpoint> save_pop;
    if (params.save_freq > 0) save_pop.reset(new SavePopCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<SaveFirstFrontCheckpoint> save_front;
    if (params.save_freq > 0) save_front.reset(new SaveFirstFrontCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<Hypervolume> hvol(new Hypervolume(1, params.save_dir.second, Hypervolume::TERMINATION, params.max_gen_hvol));
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));

    std::string mail_subj("Hypervolume of front from Geonamica optimiser");
    boost::shared_ptr<MailCheckpoint> mail;
    if (params.save_freq > 0)
    {
        mail.reset(new MailCheckpoint(params.save_freq, hvol, mail_subj));
        if (!params.email_addresses_2_send_progress.empty())
        {
            for(std::string address: params.email_addresses_2_send_progress)
            {
                mail->addAddress(address);
            }
        }
    }


    boost::shared_ptr<SignalCheckpoint> signal_handler(new SignalCheckpoint(SIGINT));
    boost::shared_ptr<MetricQtSignal> hvol_plot_signal(new MetricQtSignal(hvol));
    boost::shared_ptr<FrontQtSignal> front_plot_signal(new FrontQtSignal);
//    boost::shared_ptr<QtDialogueCheckpoint> ask_continue(new QtDialogueCheckpoint);

    if (params.save_freq > 0) optimiser.add_checkpoint(save_pop);
    if (params.save_freq > 0) optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
    if (!params.email_addresses_2_send_progress.empty())
    {
        if (params.save_freq > 0) optimiser.add_checkpoint(mail);
    }
    optimiser.add_checkpoint(maxgen);
    optimiser.add_checkpoint(signal_handler);
    optimiser.add_checkpoint(hvol_plot_signal);
    optimiser.add_checkpoint(front_plot_signal);
//    optimiser.add_checkpoint(ask_continue);
    QObject::connect(hvol_plot_signal.get(), SIGNAL(nextMetricValue(int, double)), thread, SIGNAL(nextHypervolumeMetric(int, double)));
    QObject::connect(front_plot_signal.get(), SIGNAL(nextFront(int, QVector<std::pair<double, double> >)), thread, SIGNAL(nextFront(int, QVector<std::pair<double, double> >)));
}

template void
createCheckpointsQtGUI(NSGAII<std::mt19937> & optimiser, GeonamicaPolicyParameters & params, OptimiserWorker * thread );