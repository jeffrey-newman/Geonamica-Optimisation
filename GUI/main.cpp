#include <iostream>
#include <thread>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>

#include "ParallelEvaluator.hpp"
#include "NSGAII.hpp"
#include "../GeonamicaPolicyOptimiser.hpp"
#include "../GeonamicaPolicyParameters.hpp"
#include "../GeonamicaPolicyCheckpoints.hpp"
#include "../GeonamicaPolicyPostProcess.hpp"


#ifdef WITH_QT
#include <QApplication>
#include <QCommandLineParser>
#include "MainWindow.hpp"
#endif





int main(int argc, char *argv[])
{


    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;
    bool using_mpi = false;
    if (world.size() > 1) using_mpi = true;



    if (world.rank() == 0)
    {

        CmdLinePaths cfg_file;
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("cfg-file", boost::program_options::value<std::string>(&cfg_file.first)->default_value(""), "Configuration file for optimisation")
            ("no-gui", "Run as command line executable without GUI")
            ("test", "Test the optimisation, but do not run it")
            ;

        boost::program_options::positional_options_description p;
        p.add("cfg-file", -1);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        boost::program_options::notify(vm);


        if (vm.count("help")) {
            std::cout << desc << "\n";
            return EXIT_SUCCESS;
        }


        if (vm.count("no-gui"))
        {


            if (cfg_file.first.empty())
            {
                std::cerr << "Must specify path to cfg file when using --no-gui\n";
                if (using_mpi) env.abort(EXIT_FAILURE);
                return EXIT_FAILURE;
            }
            else
            {
                bool success = pathify(cfg_file);
                if (!success)
                {
                    std::cerr << "Must specify path to existing cfg file when using --no-gui\n";
                    if (using_mpi) env.abort(EXIT_FAILURE);
                    return EXIT_FAILURE;
                }
            }
            LoadParameters parameter_loader;
            GeonamicaPolicyParameters params;
            std::this_thread::sleep_for(std::chrono::seconds(world.rank()));
            parameter_loader.processOptions(cfg_file.second.string(), params);
            params.evaluator_id = world.rank();

            if (vm.count("test"))
            {
                // When testing, we want all output to go to the testing directory
                params.working_dir.first = params.test_dir.first;
//                params.working_dir.second = params.test_dir.second;
                params.save_dir.first = params.test_dir.first;
//                params.save_dir.second = params.test_dir.second;
                params.pop_size = 3;
            }
            parameter_loader.checkNeededDirectories(params);

            if (using_mpi) boost::mpi::broadcast(world, params, 0);

            GeonamicaOptimiser geon_eval(params);

            boost::shared_ptr<boost::timer::auto_cpu_timer> timer(nullptr);
            boost::filesystem::path timing_fname = params.save_dir.second / "GeonOptTiming.txt";
            std::ofstream timer_fs(timing_fname.c_str());
            if (timer_fs.is_open())
            {
                timer.reset(new boost::timer::auto_cpu_timer(timer_fs, 3));
            }
            else
            {
                std::cerr << "Error: Could not open file for writing time elapsed for search, using std::cout";
                timer.reset(new boost::timer::auto_cpu_timer(3));
            }

            //create evaluator server
            boost::filesystem::path eval_log = params.save_dir.second / "evaluation_timing.log";
            std::ofstream eval_strm(eval_log.c_str());
            ParallelEvaluatePopServerNonBlocking eval_server(env, world, geon_eval.getProblemDefinitions());
            if (eval_strm.is_open())
            {
                eval_server.log(ParallelEvaluatorBase::LVL1, eval_strm);
            }

            // The random number generator
            typedef std::mt19937 RNG;
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            RNG rng(seed);

            if (vm.count("test"))
            {
                ProblemDefinitionsSPtr problem_defs = geon_eval.getProblemDefinitions();
                IndividualSPtr max_dvs(new Individual(problem_defs));
                max_dvs->setIntDVs(problem_defs->int_upperbounds);
                max_dvs->setRealDVs(problem_defs->real_upperbounds);
                IndividualSPtr min_dvs(new Individual(problem_defs));
                min_dvs->setIntDVs(problem_defs->int_lowerbounds);
                min_dvs->setRealDVs(problem_defs->real_lowerbounds);

                std::vector<double> objectives;
                std::vector<double> constraints;

                boost::filesystem::path save_min_dv_dir = params.test_dir.second / "min-dvs";
                if (!boost::filesystem::exists(save_min_dv_dir)) boost::filesystem::create_directories(save_min_dv_dir);
                std::tie(objectives, constraints) = geon_eval(min_dvs->getRealDVVector(), min_dvs->getIntDVVector(), save_min_dv_dir);
                min_dvs->setObjectives(objectives);
                min_dvs->setConstraints(constraints);
                std::cout << "All dvs min value: " << std::endl;
                std::cout << *min_dvs << std::endl;

                boost::filesystem::path save_max_dv_dir = params.test_dir.second / "max-dvs";
                if (!boost::filesystem::exists(save_max_dv_dir)) boost::filesystem::create_directories(save_max_dv_dir);
                std::tie(objectives, constraints) = geon_eval(max_dvs->getRealDVVector(), max_dvs->getIntDVVector(), save_max_dv_dir);
                max_dvs->setObjectives(objectives);
                max_dvs->setConstraints(constraints);
                std::cout << "All dvs max value: " << std::endl;
                std::cout << *max_dvs << std::endl;

                boost::filesystem::path save_rand_dv_dir = params.test_dir.second / "random-dvs";
                if (!boost::filesystem::exists(save_max_dv_dir)) boost::filesystem::create_directories(save_rand_dv_dir);
                PopulationSPtr pop(new Population);
                pop =
                    intialisePopulationRandomDVAssignment(params.pop_size, geon_eval.getProblemDefinitions(), rng);
                //Postprocess the results
                postProcessResults(geon_eval, pop, params, save_rand_dv_dir, false);

            }
            else
            {
                // The optimiser
                boost::scoped_ptr<NSGAII<RNG> > optimiser;
                if (using_mpi)
                {
                    optimiser.reset(new NSGAII<RNG>(rng, eval_server));
                }
                else
                {
                    optimiser.reset(new NSGAII<RNG>(rng, geon_eval));
                }

                if (eval_strm.is_open())
                {
                    optimiser->log(eval_strm, eval_log, NSGAII<RNG>::LVL1);
                }

                // Add the checkpoints
                createCheckpoints(*optimiser, params);


                PopulationSPtr pop(new Population);
                if (params.restart_pop_file.first == "no_seed")
                {
                    pop =
                        intialisePopulationRandomDVAssignment(params.pop_size, geon_eval.getProblemDefinitions(), rng);
                }
                else
                {
                    pop = restore_population(params.restart_pop_file.second);
                }
                optimiser->getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

                optimiser->initialiseWithPop(pop);
                optimiser->run();

                //Postprocess the results
                postProcessResults(geon_eval, pop, params);
            }


            timer.reset((boost::timer::auto_cpu_timer *) nullptr);
            if (timer_fs.is_open())
            {
                timer_fs.close();
            }
        }
        else
        {

#ifdef WITH_QT
            QApplication a(argc, argv);
            QCoreApplication::setApplicationName("Geonamica Optimisation Utility");
            QCoreApplication::setOrganizationName("BNHCRC / EnvirocareInformatics / University of Adelaide");
            QCoreApplication::setApplicationVersion("0.1 Alpha");

            qRegisterMetaType<QVector<std::pair<double, double> >>();

            //Use GUI
            MainWindow main_win;
            main_win.show();

            if (!cfg_file.first.empty())
            {
                pathify(cfg_file);
                QString q_cfg_file = QString::fromStdString(cfg_file.second.string());
                main_win.openFile(q_cfg_file);
            }
            return a.exec();
#else
            std::cerr << "Must specify --no-gui option when optimiser has not been compiled with GUI frontend\n";
            return EXIT_FAILURE;
#endif
        }

    }
    else
    {
        //load parameters
        try
        {
            GeonamicaPolicyParameters params;
            boost::mpi::broadcast(world, params, 0);
//            std::cout << "received params\n";
            params.evaluator_id = world.rank();
            //Sleep the threads so that they do not all try and create the same working directory at once, which could potentially cause havoc. This creation usually occurs in the evaluatior constructor but could also be placed in the command line option parser.
            std::this_thread::sleep_for(std::chrono::seconds(world.rank()));
            std::string log_file_name = "worker_" + std::to_string(world.rank()) + "_timing.log";
            boost::filesystem::path eval_log = params.save_dir.second / log_file_name;
            std::ofstream eval_strm(eval_log.c_str());
            boost::shared_ptr<GeonamicaOptimiser> geon_eval(new GeonamicaOptimiser(params));
            ParallelEvaluatePopClientNonBlocking eval_client(env, world, geon_eval->getProblemDefinitions(), *geon_eval);
            eval_client();
        }
        catch(std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
            std::cerr << "Unrecoverable; closing down optimiser worker" << world.rank() << "\n";
            return EXIT_FAILURE;
        }
        catch(...)
        {
            std::cerr << "Unknown error!" << "\n";
            std::cerr << "Unrecoverable; closing down optimiser worker" << world.rank() << "\n";
            return EXIT_FAILURE;
        }
    }

}
