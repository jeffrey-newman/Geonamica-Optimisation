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
        std::cout << "This is Jeffrey Newman's Geonamica Optimiser" << std::endl;
        if (using_mpi) std::cout << "Running in parallel mode using MPI" << std::endl;
        else std::cout << "Runnning in serial mode" << std::endl;

        CmdLinePaths cfg_file;
        CmdLinePaths gen_pop_file;
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("cfg-file", boost::program_options::value<std::string>(&cfg_file.first)->default_value(""), "Configuration file for optimisation")
            ("no-gui", "Run as command line executable without GUI")
            ("test", "Test the optimisation, but do not run it. Must be used in combination with --no-gui")
            ("postprocess", "Run the evaluation and save it for the population in the reseed file the ressed file is specified in the cfg file. Must be used in combination with --no-gui")
                ("generate-pop", boost::program_options::value<std::string>(&gen_pop_file.first), "Generate a random population then quit, saving population to file specified")
            ;

        boost::program_options::positional_options_description p;
        p.add("cfg-file", -1);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        boost::program_options::notify(vm);


        if (vm.count("help")) {
            std::cout << desc << "\n";
            if (using_mpi) env.abort(EXIT_SUCCESS);
            return EXIT_SUCCESS;
        }

        if (vm.count("generate-pop"))
        {
            std::cout << "You have requested that I generate a random population,\n"
                " based on the optimisation problem configurations you gave me" << std::endl;
            if (cfg_file.first.empty())
            {
                std::cerr << "I've reached a problem. You must specify a path to a cfg file when using --generate-pop" << std::endl;
                if (using_mpi) env.abort(EXIT_FAILURE);
                return EXIT_FAILURE;
            }
            else
            {
                bool success = pathify(cfg_file);
                if (!success)
                {
                    std::cerr << "I've reached a problem. You must specify a path to an existing cfg file when using --generate-pop" << std::endl;
                    if (using_mpi) env.abort(EXIT_FAILURE);
                    return EXIT_FAILURE;
                }
            }

            // The random number generator
            typedef std::mt19937 RNG;
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            RNG rng(seed);

            //Getting problem characteristics
            LoadParameters parameter_loader;
            GeonamicaPolicyParameters params;
            std::this_thread::sleep_for(std::chrono::seconds(world.rank()));
            parameter_loader.processOptions(cfg_file.second.string(), params);
            parameter_loader.checkNeededDirectories(params);
            GeonamicaOptimiser geon_eval(params);

            //Generate and save the population
            PopulationSPtr pop(new Population);
            pop = intialisePopulationRandomDVAssignment(params.pop_size, geon_eval.getProblemDefinitions(), rng);
            gen_pop_file.second = boost::filesystem::path(gen_pop_file.first);
            print(pop, gen_pop_file.second);
            if (using_mpi) env.abort(EXIT_SUCCESS);
            std::cout << "I've finished generating that random population you asked for" << std::endl;
            return EXIT_SUCCESS;
        }

        if (vm.count("no-gui"))
        {
            std::cout << "You've asked me to run in command-line mode. I will not display my GUI" << std::endl;

            if (cfg_file.first.empty())
            {
                std::cerr << "I've reached a problem. You must specify a path to a cfg file when using --no-gui" << std::endl;
                if (using_mpi) env.abort(EXIT_FAILURE);
                return EXIT_FAILURE;
            }
            else
            {
                bool success = pathify(cfg_file);
                if (!success)
                {
                    std::cerr << "I've reached a problem. Must specify a path to an existing cfg file when using --no-gui" << std::endl;
                    if (using_mpi) env.abort(EXIT_FAILURE);
                    return EXIT_FAILURE;
                }
            }

            std::cout << "I'm loading the optimisation problem configuration now" << std::endl;
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

            std::cout << "I'm read the configuration. Just setting a few things up now" << std::endl;
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

            std::cout << "Things are all set up now. Ready to do some work!" << std::endl;

            if (vm.count("test"))
            {
                std::cout << " Testing the optimisation configuration, as you requested...." << std::endl;
                ProblemDefinitionsSPtr problem_defs = geon_eval.getProblemDefinitions();
                IndividualSPtr max_dvs(new Individual(problem_defs));
                max_dvs->setIntDVs(problem_defs->int_upperbounds);
                max_dvs->setRealDVs(problem_defs->real_upperbounds);
                IndividualSPtr min_dvs(new Individual(problem_defs));
                min_dvs->setIntDVs(problem_defs->int_lowerbounds);
                min_dvs->setRealDVs(problem_defs->real_lowerbounds);

                PopulationSPtr pop(new Population);
                pop =
                    intialisePopulationRandomDVAssignment(params.pop_size, geon_eval.getProblemDefinitions(), rng);
                pop->push_back(max_dvs);
                pop->push_back(min_dvs);
                //Postprocess the results
                optimiser->postProcess(pop, params.test_dir.second);

            }
            else if(vm.count("postprocess"))
            {
                std::cout << " Postprocessing the population file you gave me...." << std::endl;
                if (not(params.restart_pop_file.first == "no_seed" || params.restart_pop_file.first.empty()))
                {
                    ProblemDefinitionsSPtr problem_defs = geon_eval.getProblemDefinitions();
                    PopulationSPtr pop(new Population);
                    pop = restore_population(params.restart_pop_file.second, problem_defs);
                    optimiser->postProcess(pop, params.save_dir.second);
                }
                else
                {
                    std::cerr << "I've reached a problem. Must specify a path to a population file when using --postprocess within the configuration" << std::endl;
                    if (using_mpi) env.abort(EXIT_FAILURE);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                if (eval_strm.is_open())
                {
                    optimiser->log(eval_strm, eval_log, NSGAII<RNG>::LVL1);
                }

                // Add the checkpoints
                createCheckpoints(*optimiser, params);


                PopulationSPtr pop(new Population);
                if (params.restart_pop_file.first == "no_seed" || params.restart_pop_file.first.empty())
                {
                    std::cout << "Creating the population" << std::endl;
                    pop =
                        intialisePopulationRandomDVAssignment(params.pop_size, geon_eval.getProblemDefinitions(), rng);
                }
                else
                {
                    std::cout << "Restoring the population" << std::endl;
                    pop = restore_population(params.restart_pop_file.second, geon_eval.getProblemDefinitions());
                }
                optimiser->getIntMutationOperator().setMutationInverseDVSize(pop->at(0));
                optimiser->initialiseWithPop(pop);

                std::cout << " Running the optimisation now...." << std::endl;
                optimiser->run();

                //Postprocess the results
                std::cout << " Finished running the optimisation. Now postprocessing" << std::endl;
                optimiser->postProcess(pop, params.save_dir.second);
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
        if (using_mpi) env.abort(EXIT_SUCCESS);
    }
    else
    {
        std::cout << "This is a worker for the Geonamica optimisation, reporting for duty!" << std::endl;
        //load parameters
        try
        {
            GeonamicaPolicyParameters params;
            boost::mpi::broadcast(world, params, 0);
//            std::cout << "received params\n";
            params.evaluator_id = world.rank();
            std::cout << "My worker ID is " << params.evaluator_id;
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
        return EXIT_SUCCESS;
    }

}
