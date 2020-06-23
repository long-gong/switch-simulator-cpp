//
// Created by Long Gong on 9/27/16.
//

#ifndef LWS_H
#define LWS_H

#include <iostream>
#include <iomanip>
#include <cmath>
#include <queue> // for FIFO queue
#include <vector>
#include <string>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <cassert>
#include <limits>
#include <chrono>
#include "cxxopts.hpp"
#include "json.hpp"
#include "random_variable.hpp"
#include "./HdrHistogram_c/src/hdr_histogram.h"

/* ---------------------------------------------------------------------------------- */
/*                                  Macros                                            */
/* ---------------------------------------------------------------------------------- */


/*
 *TODO: change macros to enum
 */
/* special constant */
#define LWS_NOARRIVAL -1
#define LWS_UNMATCHED -1
#define LWS_NODEPARTURE -1
#define LWS_MATCHED 1

/* inject options */
#define LWS_BERNOULLI_IID 0
#define LWS_ON_OFF_BURST 1

/* traffic matrix options */
#define LWS_UNIFORM 0
#define LWS_LOG_DIAGONAL 1
#define LWS_QUASI_DIAGONAL 2
#define LWS_DIAGONAL 3
#define LWS_ALL_MODELS -1


/* logging options */
#define LWS_SCREEN_ONLY 0
#define LWS_FILE_ONLY 1
#define LWS_BOTH 2
#define LWS_OUT_WIDTH 16
#define LWS_PRECISION 4
#define LWS_OUT_FREQ 32


/* percentile setting */
#define LWS_SIGNIFICANT_BITS 3
#define LWS_MIN_VALUE 1
#define LWS_MAX_VALUE 3600000000LL

/* Simulation options */
#define LWS_THROUGHPUT -1
#define LWS_DELAY_VS_LOAD 0
#define LWS_DELAY_VS_BURST_SIZE 1

#define LWS_DEF_LOAD_TH 0.99
#define LWS_DEF_PORT_NUM 64
#define LWS_DEF_FRAMES 1000


/* maximum number of multiple values for the loads or burst sizes */
#define LWS_MAX_MULTI 64

// /* useful macros for warping common used functions */
//#define ALLOC(type, num)	\
//    ((type *) malloc(sizeof(type) * (num)))
//
//#define REALLOC(type, obj, num)	\
//    (obj) ? ((type *) realloc((char *) obj, sizeof(type) * (num))) : \
//    ((type *) malloc(sizeof(type) * (num)))
//
//#define FREE(obj)		\
//    ((obj) ? (free((char *) (obj)), (obj) = 0) : 0)



/* ---------------------------------------------------------------------------------- */
/*                             Objects                                                */
/* ---------------------------------------------------------------------------------- */
struct packet_t {
    int src;
    int dest;
    int time;
    packet_t(): src(0), dest(0), time(0) {}
    packet_t(int s, int d, int t): src(s), dest(d), time(t) {
        //std::cout << "create a packet" << std::endl;
    }
    // int index; /*! needed by queue proportional sampling to eliminate hash table */
};

typedef struct packet_t  packet_t;

struct burst_t {
    int on;
    int dest;
    burst_t(): on(0), dest(LWS_NOARRIVAL){}
    burst_t(int o, int d): on(o), dest(d) {}
};

typedef struct burst_t burst_t;

typedef std::vector<std::vector<std::queue<std::shared_ptr<packet_t> > > > queue_t;

typedef struct { /* lightweight switch parameters object */
    std::string name;   		/* name of the simulator */
    int type;              /* type of the simulation */

    int model_option; 		/* traffic model option (default = uniform) */
    std::string model_name; 		/* traffic model name */

    int inject_option; 		/* injection option (default = Bernoulli) */
    std::string inject_name; 		/* injection name */

    int N;				    /* number of ports */
    int MAX_M;  			/* number of macroframes */

    unsigned seed;          /* seed for traffic generator */

    int verbosity;         /* printing controller */

    std::string logging_name;    /* logging file name */

    double average_burst_length;  /* current average burst length for geometric burst */
    std::vector<double> abls; /* burst size to be simulated */
    int num_abls; /* number of average burst size to be simulated */
    double prob_on; /* probability for on to off */
    double prob_off; /* probability for off to on */

    double minimum_load; /* minimum load */
    double maximum_load; /* maximum load */


    double load_step; /* load step */

    int accept_buffer; /* buffer size for accepting */

    std::vector<double> loads; /* traffic loads to be simulated */
    int num_loads; /* number of loads to be simulated */
    double load; /* current simulated normalized load */

    //double foo_bar[8]; /* reserved for future usage */
} lws_param_t;

typedef struct {/* lightweight switch status */
    std::vector<int> A; /* arrivals */
    /* int *D; */ /* departures */
    int cur_time; /* current time slot */

    // std::vector<std::shared_ptr<burst_t> > burst; /* burst status */
    queue_t B; /* Virtual output queue status */

    /* queue length for each virtual output queue,
     * it is used for scheduling algorithms that cannot
     * guarantee 100% throughput. Because if the scheduling
     * algorithm is not stable, the virtual output queue would
     * be with length infinite, therefore, we can not hold packets
     * in the queues. In that case, we will just store how many packets
     * are in each queues.
     * */
    std::vector<std::vector<int> > Q;
    std::vector<int> S; /* schedule */
} lws_status_t;


typedef struct {
    int max_delay;			/* max delay */
    double mean_delay;  		/* mean delay or queue length */
    double mean_delay_count;	/* used in mean delay calc */
    double sum_delay;           /* sum of delays from all VOQs */

    double T_arr;  			/* total arrival */
    double T_dep;  			/* total departure */

    std::ofstream logger;              /* logging file pointer */

    struct hdr_histogram *histogram; /* for percentile delay */

    std::chrono::high_resolution_clock::time_point start; /* start time */

    // for comparing fairness
    std::vector<std::vector<double> > sum_delay_per_voq;
    std::vector<std::vector<double> > delay_counter_per_voq;


    double foo_bar[8];    /* reserved for future usage */

} lws_inst_t;


static const std::string MODEL_NAMES[4] = {
        "uniform",
        "log-diagonal",
        "quasi-diagonal",
        "diagonal"
};

static const std::string INJECTION_NAMES[2] = {
        "Bernoulli",
        "on-off-burst"
};

static const std::string DEF_LOGGING_NAME = "../logging/default_logging.txt";


class LWS_SwitchCore {
public:
    cxxopts::Options options;
    lws_param_t params;
    lws_status_t status;
    lws_inst_t instruments;
    enum STATE {
        CREATED,
        PARSER_CONFIGED,
        PARSED,
        INITED
    };
    STATE s;
    int show_counter;

    LWS_SwitchCore(char *name): options(name, " - command line options"){
        s = CREATED;
        show_counter = 0;
    }
    void config(){
        try {
            options.add_options()
                    ("p,port", "port number", cxxopts::value<int>(), "N")
                    ("m,frames", "frame number", cxxopts::value<int>(), "N")
                    ("t,traffic", "traffic matrix model (u, l, q, d)", cxxopts::value<char>(), "C")
                    ("l,loads", "traffic loads", cxxopts::value<std::vector<double> >(), "LOAD")
                    ("v,verbosity", "verbosity level", cxxopts::value<int>(), "N")
                    ("b,burst", "burst size", cxxopts::value<std::vector<double> >(), "BUSRT_SIZE")
                    ("s,seed", "seed for traffic generator", cxxopts::value<unsigned>(), "SEED")
                    ("f,file", "log file", cxxopts::value<std::string>(), "FILE")
                    ("T,Throughput", "Measure throughput under which load",cxxopts::value<double>(), "LOAD")
                    ("n,name", "simulator name", cxxopts::value<std::string>(), "NAME")
                    ("B,Buffer", "buffer size for accepting", cxxopts::value<int>(), "N")
                    ("h,help", "Print help");
        }catch (const cxxopts::OptionException& e){
            std::cerr << "Error while configuring parser: " << e.what() << std::endl;
            exit(1);
        }
        s = PARSER_CONFIGED;
    }
    void parsing(int argc, char *argv[]){
            //std::cout << "argc = " << argc << std::endl;
        int argc_bk = argc; /* pay attention */

        params.type = LWS_DELAY_VS_LOAD;
        params.inject_option = LWS_BERNOULLI_IID;
        params.model_option = LWS_ALL_MODELS;
        params.name = "Simulator";

        params.seed = std::chrono::system_clock::now().time_since_epoch().count(); /* default seed */
        try{
            options.parse(argc, argv);/* Note that, here parse will change the value of argc and argv */
            //std::cout << options.count("h") << std::endl;
            if (options.count("h") || argc_bk == 1) {std::cout << options.help() << std::endl; exit(0); }
            if (options.count("n")) params.name = options["n"].as<std::string>();
            if (options.count("p")) params.N = options["p"].as<int>();
            else params.N = LWS_DEF_PORT_NUM;
            if (options.count("m")) params.MAX_M = options["m"].as<int>();
            else params.MAX_M = LWS_DEF_FRAMES;
            params.MAX_M *= params.N * params.N;
            if (options.count("b")) {
                params.abls = options["b"].as<std::vector<double> >();
                params.type = LWS_DELAY_VS_BURST_SIZE;
            }
            if (options.count("l")) params.loads = options["l"].as<std::vector<double> >();
            else {
                if (params.type == LWS_BERNOULLI_IID)
                    params.loads = {0.99, 0.95, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
                else if (params.type == LWS_ON_OFF_BURST)
                {
                    throw cxxopts::OptionException("at least one traffic load value should be entered, when you use -b");
                }
            }
            if (options.count("s")) params.seed = options["s"].as<unsigned>();

            if (options.count("B")) params.accept_buffer = options["B"].as<int>();
            else params.accept_buffer = LWS_DEF_PORT_NUM;

            if (params.abls.size() == 0) params.inject_option = LWS_BERNOULLI_IID;
            else params.inject_option = LWS_ON_OFF_BURST;
            params.inject_name = INJECTION_NAMES[params.inject_option];
            if (options.count("v")) params.verbosity = options["v"].as<int>();
            else params.verbosity = 0;
            if (options.count("f")) params.logging_name = options["f"].as<std::string>();
            else params.logging_name = DEF_LOGGING_NAME;
            if (options.count("t")) {
                switch(options["t"].as<char>()){
                    case 'u':
                        params.model_option = LWS_UNIFORM;
                        break;
                    case 'l':
                        params.model_option = LWS_LOG_DIAGONAL;
                        break;
                    case 'q':
                        params.model_option = LWS_QUASI_DIAGONAL;
                        break;
                    case 'd':
                        params.model_option = LWS_DIAGONAL;
                        break;
                    default:
                        params.model_option = LWS_ALL_MODELS;
                }
            }
            if (params.model_option != LWS_ALL_MODELS) params.model_name = MODEL_NAMES[params.model_option];
            if (options.count("T")) {
                params.type = LWS_THROUGHPUT;
                params.loads.clear();
                params.loads.push_back(options["T"].as<double>());
            }
            //
        }catch (const cxxopts::OptionException &e){
            std::cerr << "while parsing parameters: " << e.what() << "\n\n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
        s = PARSED;
    }
    void print_params() const {
        nlohmann::json j2 = {
                {"name", params.name},
                {"type", params.type},
                {"model_option", params.model_option},
                {"model_name", params.model_name},
                {"injection_option", params.inject_option},
                {"injection_name", params.inject_name},
                {"port_number", params.N},
                {"frame_number", params.MAX_M},
                {"logging_file", params.logging_name},
                {"loads", params.loads},
                {"abls", params.abls},
        };
        std::cout << j2.dump(4) << std::endl;
    }
    void print_status() const {
        if (s != INITED) {std::cerr << "You have not inited, please init first" << std::endl; return;}
        nlohmann::json j = {
                {"arrivals", status.A },
                {"schedule", status.S },
                {"VOQs", status.Q },
                {"current_time", status.cur_time}
        };
        std::cout << j.dump(4) << std::endl;
    }
    void print_instruments() const {
        if (s != INITED) {std::cerr << "You have not inited, please init first" << std::endl; return;}
        nlohmann::json j = {
                {"arrivals", instruments.T_arr},
                {"departures", instruments.T_dep},
                {"mean_delay", instruments.mean_delay},
                {"max_delay", instruments.max_delay},
                {"delay_counter", instruments.mean_delay_count},
                {"sum_delay", instruments.sum_delay}
        };
        std::cout << j.dump(4) << std::endl;
        hdr_percentiles_print(
                instruments.histogram,
                stdout,
                5,
                1.0,
                CLASSIC);
    }
    void print_all() const {
        print_params();

        print_status();
        print_instruments();
    }
    void init(int argc, char *argv[]) {
        if (s == CREATED) {config(); parsing(argc, argv);}
        else if (s == PARSER_CONFIGED) {parsing(argc, argv);}
        init();
    }
    void init() {
        if (s == CREATED) {
            std::cerr << "you have not configure your switch, please call configure first! " << std::endl;
            return;
        }
        status.A.resize(params.N);
        for (int i = 0;i < params.N;++ i) status.A[i] = LWS_NOARRIVAL; /* initilized as no arrivals */
        //std::cout << "A" << std::endl;
        /*
        if (params.inject_option == LWS_ON_OFF_BURST) {
            status.burst.resize(params.N);
            for (int i  = 0;i < params.N; ++ i) status.burst[i] = std::make_shared<burst_t>(0, LWS_NOARRIVAL);
        }
        */

        //std::cout << "b" << std::endl;
        status.cur_time = 0;
        status.Q.resize(params.N);
        //std::cout << "Q" << std::endl;
        for (int i = 0;i < params.N;++ i) {
            status.Q[i].resize(params.N);
            for (int k = 0;k < params.N;++ k) status.Q[i][k] = 0;
        }
        //std::cout << "Q" << std::endl;
        status.S.resize(params.N);
        for (int i = 0;i < params.N;++ i) status.S[i] = i; /* initilze as identical matching */
        //std::cout << "S" << std::endl;
        if (params.type != LWS_THROUGHPUT) { /* fixed */
            status.B.resize(params.N);
            for (int i = 0;i < params.N;++ i) status.B[i].resize(params.N);
        }
        //std::cout << "B" << std::endl;

        status.cur_time = 0;

        instruments.mean_delay_count = 0;
        instruments.mean_delay = 0;
        instruments.max_delay = 0;
        instruments.sum_delay = 0;
        instruments.T_arr = 0;
        instruments.T_dep = 0;
        instruments.logger.open(params.logging_name);

        instruments.start = std::chrono::high_resolution_clock::now(); // starting time
                                                                       // 
        instruments.sum_delay_per_voq.resize(params.N);
        instruments.delay_counter_per_voq.resize(params.N);
        for (int i = 0;i < params.N;++ i) {
            instruments.sum_delay_per_voq[i].resize(params.N);
            instruments.delay_counter_per_voq[i].resize(params.N);
            for (int k = 0;k < params.N;++ k) {
                instruments.sum_delay_per_voq[i][k] = 0;
                instruments.delay_counter_per_voq[i][k] = 0;
            }
        }

        hdr_init(
                LWS_MIN_VALUE,
                LWS_MAX_VALUE,
                LWS_SIGNIFICANT_BITS,
                &(instruments.histogram)
        );

        s = INITED;
    }
    void reset(){
        std::cout << "reset:: start ..." << std::endl;
        if (s != INITED) { std::cerr << "You have not init, please init first" << std::endl; return; }

        std::cout << "reset:: A ..." << std::endl;
        assert(params.N == status.A.size());
        for (int i = 0;i < params.N;++ i) status.A[i] = LWS_NOARRIVAL;

        /*
        if (params.inject_option == LWS_ON_OFF_BURST) {
            std::cout << "reset:: burst ..." << std::endl;
            assert(params.N == status.burst.size());
            for (int i  = 0;i < params.N; ++ i) status.burst[i] = std::make_shared<burst_t>(0, LWS_NOARRIVAL);
        }
        */

        std::cout << "reset:: Q ..." << std::endl;
        assert(params.N == status.Q.size());
        for (int i = 0;i < params.N;++ i) {
            assert(params.N == status.Q[i].size());
            for (int k = 0;k < params.N;++ k) status.Q[i][k] = 0;
        }

        if (params.type != LWS_THROUGHPUT) { /* fixed */
            std::cout << "reset:: B ..." << std::endl;
            assert(params.N == status.B.size());
            for (int i = 0;i < params.N;++ i) {
                assert(params.N == status.B[i].size());
                for (int k = 0;k < params.N;++ k) {
                    while (!status.B[i][k].empty()) status.B[i][k].pop();
                }
            }
        }

        std::cout << "reset:: S ..." << std::endl;
        assert(status.S.size() == params.N);
        for (int i = 0;i < params.N;++ i) status.S[i] = i;

        std::cout << "reset:: instruments ..." << std::endl;

        instruments.mean_delay_count = 0;
        instruments.mean_delay = 0;
        instruments.max_delay = 0;
        instruments.sum_delay = 0;
        instruments.T_arr = 0;
        instruments.T_dep = 0;

        // for fairness
        for (int i = 0;i < params.N;++ i) {
            for (int k = 0;k < params.N;++ k) {
                instruments.sum_delay_per_voq[i][k] = 0;
                instruments.delay_counter_per_voq[i][k] = 0;
            }
        }

        std::cout << "reset histogram ..." << std::endl;

        hdr_reset(instruments.histogram);
    }
    void destroy() {
        if (s != INITED) return;
        status.A.clear();
        // status.burst.clear();
        for (int i = 0;i < params.N;++ i) {
            status.Q[i].clear();
            if (params.type != LWS_THROUGHPUT)
            {
                for (int k = 0;k < params.N;++ k)
                    while (!status.B[i][k].empty()) status.B[i][k].pop();
                status.B[i].clear();
            }
        }
        status.Q.clear();
        status.B.clear();
        status.S.clear();

        instruments.logger.close();
        instruments.sum_delay_per_voq.clear();
        instruments.delay_counter_per_voq.clear();

        if (instruments.histogram != NULL) free(instruments.histogram);


        s = CREATED;
    }
    void show(std::ostream& out=std::cout, bool reset_counter=false){

        if (show_counter == 0) show_header(out);


        out       << std::setw(6)
                  << std::left
                  << std::fixed
                  << std::setprecision(2)
                  << params.load
                  << std::setw(13)
                  << std::left
                  << params.model_option;

        if (params.inject_option == LWS_ON_OFF_BURST)
             out      << std::setw(11)
                      << std::left
                      << std::fixed
                      << std::setprecision(2)
                      << params.average_burst_length;

        out           << std::setw(12)
                      << std::left
                      << std::fixed
                      << std::setprecision(LWS_PRECISION)
                      << ((instruments.T_arr == 0)? 0:(instruments.T_dep / instruments.T_arr))
                      << std::setw(16)
                      << std::left
                      << ((instruments.mean_delay_count == 0)? -1: (instruments.sum_delay / instruments.mean_delay_count));

        out           << std::setw(12)
                      << std::left
                      << instruments.max_delay;

        out           << std::setw(16)
                      << std::left
                      << std::fixed
                      << std::setprecision(LWS_PRECISION)
                      << hdr_mean(instruments.histogram) - 1;

        out           << std::setw(14)
                      << std::left
                      << hdr_max(instruments.histogram) - 1;

        out           << std::setw(15)
                      << std::left
                      << std::fixed
                      << std::setprecision(LWS_PRECISION)
                      << hdr_stddev(instruments.histogram);
        out           << std::setw(12)
                      << std::left
                      << hdr_value_at_percentile(instruments.histogram,90) - 1
                      << std::setw(12)
                      << std::left
                      << hdr_value_at_percentile(instruments.histogram,95) - 1
                      << std::setw(12)
                      << std::left
                      << hdr_value_at_percentile(instruments.histogram,99) - 1
                      << std::endl;

        if (reset_counter) show_counter = 0;
        else ++ show_counter;

    }
    void show_header(std::ostream& out=std::cout, bool update_counter=false){


            out << std::setw(6)
                << std::left
                << "#Load"
                << std::setw(13)
                << std::left
                << "Traffic-Mode";
        if (params.inject_option == LWS_ON_OFF_BURST)
            out     << std::setw(11)
                    << std::left
                    << "Burst-Size";

        out     << std::setw(12)
                << std::left
                << "Throughput"
                << std::setw(16)
                << std::left
                << "Mean-Delay"
                << std::setw(12)
                << std::left
                << "Max-Delay"
                << std::setw(16)
                << std::left
                << "Mean-Delay-HDR"
                << std::setw(14)
                << std::left
                << "Max-Delay-HDR"
                << std::setw(15)
                << std::left
                << "Delay-STD-HDR"
                << std::setw(12)
                << std::left
                << "P90-Delay"
                << std::setw(12)
                << std::left
                << "P95-Delay"
                << std::setw(12)
                << std::left
                << "P99-Delay"
                << std::endl;

        if (update_counter) ++ show_counter;
    }
//    void single_run_(unsigned seed, Scheduler *sch, std::ostream &results) {
//        if (params.model_option == LWS_ALL_MODELS) {std::cerr << "You can not run all models in single model " << std::endl; return; }
//
//        TrafficGenerator *tgen = NULL;
//        print_all();
//
//        /* create traffic generator */
//        if (params.inject_option == LWS_ON_OFF_BURST)
//            tgen = new BurstTrafficGenerator(seed, params.N, params.load, params.model_option,
//                                         params.average_burst_length);
//        else
//            tgen = new TrafficGenerator(seed, params.N, params.load, params.model_option);
//
//
//        for (int ts = 0; ts < params.MAX_M; ++ts) {
//            /* set time slot */
//            status.cur_time = ts;
//
//            SwitchEvent::packet_arrival(params, status, instruments, tgen);
//
//            /* put scheduling here */
//            // UT::identical_schedule(sw.status.S);
//            sch->run(params, status);
//            SwitchEvent::packet_departure(params, status, instruments);
//            /* put instruments printing here if necessary */
//            if (params.MAX_M - 1 == ts) show(results);
//            if ((ts > 0 && (ts % (params.MAX_M / LWS_OUT_FREQ) == 0)) || ts == params.MAX_M - 1) show(std::cout,ts == params.MAX_M - 1);
//        }
//        hdr_percentiles_print(
//                instruments.histogram,
//                stdout,  // File to write to
//                5,  // Granularity of printed values
//                1.0,  // Multiplier for results
//                CLASSIC);
//
//        delete tgen;
//        reset();
//        sch->reset(); /* reset accept * grant pointers */
//    }
//    void run(unsigned seed, Scheduler *sch, std::ostream &results) {
//        if (s != INITED) {std::cerr << "Not inited yet " << std::endl; return; }
//        show_header(results, false);
//
//        if (params.inject_option == LWS_ON_OFF_BURST) {/* burst traffic */
//            for (auto load: params.loads) {/* each load */
//                for (auto bs: params.abls) {/* each burst size */
//                    params.load = load;
//                    params.average_burst_length = bs;
//                    if (params.model_option == LWS_ALL_MODELS) {/* all models */
//                        for (int model = LWS_UNIFORM;model <= LWS_DIAGONAL;++ model) {
//                            params.model_option = model;
//                            params.model_name = MODEL_NAMES[model];
//
//                            single_run_(seed, sch, results);
//
//                        }
//                        params.model_option = LWS_ALL_MODELS;
//                    }
//                    else{
//                        // TODO
//                        single_run_(seed, sch, results);
//                    }
//                }
//            }
//        } else {
//            for (auto load: params.loads) {/* each load */
//                params.load = load;
//                if (params.model_option == LWS_ALL_MODELS) {
//
//                    for (int model = LWS_UNIFORM;model <= LWS_DIAGONAL;++ model) {
//                        params.model_option = model;
//                        params.model_name = MODEL_NAMES[model];
//                        single_run_(seed, sch, results);
//                    }
//                    params.model_option = LWS_ALL_MODELS;
//                }
//                else {
//                    // TODO
//                    single_run_(seed, sch, results);
//                }
//            }
//        }
//    }

};

class TrafficGenerator{
public:
    int N;
    double load;
   // RV rv;
    int type;

    TrafficGenerator(int port_num, double tload, int ttype): N(port_num), load(tload), type(ttype) {
    }

    int traffic_entry_(int i, RV& rv)
    {
        int j, r;
        double c, range, h;

        switch (type) {
            case LWS_UNIFORM:
            {/* uniform traffic */
                //std::cout << "i = " << i << ", uniform" << std::endl;
                r = rv.random_int(N);
                break;
            }
            case LWS_DIAGONAL:
            { /* diagonal */
                //std::cout << "diagonal" << std::endl;
                c = (double) rv.random_int(N);
                range = (((double) N) * 2.0) / 3.0;
                if (c < range) {
                    r = i;
                } else {
                    r = (i + 1) % N;
                }
                break;
            }
            case LWS_LOG_DIAGONAL:
            {/* log diagonal traffic */
                //std::cout << "log diagonal" << std::endl;
                c = (double) rv.random_int(N);
                if (c < (0.5 * ((double) N))) {
                    r = i;
                } else if (c < (0.75 * ((double) N))) {
                    r = (i + 1) % N;
                } else if (c < (0.875 * ((double) N))) {
                    r = (i + 2) % N;
                } else if (c < (0.9375 * ((double) N))) {
                    r = (i + 3) % N;
                } else if (c < (0.96875 * ((double) N))) {
                    r = (i + 4) % N;
                } else {
                    r = (i + 5) % N;
                }
                break;
            }
            case LWS_QUASI_DIAGONAL:
            {/* quasi diagonal traffic */
                //std::cout << "quasi diagonal" << std::endl;
                c = (double) rv.random_int(N);
                if (c < (0.5 * ((double) N))) {
                    r = i;
                } else {
                    h = 0.5;
                    for (j = 1; j < N; j++) {
                        h = h + (1.0 / (((double) (N - 1)) * 2.0));
                        if (c < (h * ((double) N))) {
                            r = (i + j) % N;
                            return r;
                        }
                    }
                    r = (i + (N - 1)) % N;
                }
                break;
            }
            default: {
                std::cerr << "Unsupported traffic matrix type!" << std::endl;
            }
        }
        return r;
    }

    virtual void run(std::vector<int>& A, RV& rv) {
        // std::cout << "Base Traffic Generator " << std::endl;
        // std::cout << "|A| = " << A.size() << ", N = " << N << ", load = " << load << std::endl;

        int i;
        for (i = 0;i < N;++ i) {
            if (rv.random_01() < load) {
                A[i] = traffic_entry_(i, rv);
            }
        }
    }



};

class BurstTrafficGenerator: public TrafficGenerator{
public:
    typedef struct burst_t {
        int on;
        int dest;
        burst_t(): on(0), dest(LWS_NOARRIVAL){}
        burst_t(int o, int d): on(o), dest(0){}
    } burst_t;
    std::vector<std::shared_ptr<burst_t> > burst;
    double prob_on;
    double prob_off;
    BurstTrafficGenerator(int port_num, double load, int ttype, double burst_size):\
        TrafficGenerator(port_num, load, ttype) {
        prob_on = 1.0 / (1.0 + burst_size);
        double tmp = load / (1.0 - load); /* on2off ratio */
        prob_off = tmp / (tmp + burst_size);
        burst.resize(N);
        for (int i = 0;i < N;++ i) { burst[i] = std::make_shared<burst_t>(0, LWS_NOARRIVAL); }
    }

    virtual void run(std::vector<int>& A, RV& rv) {
        // std::cout << "Derived Traffic Generator" << std::endl;
        int i, sbk;

        for (i = 0;i < N;++ i) {
            do
            {
                sbk = burst[i]->on;
                if (burst[i]->on) {/* previous circle on */
                    if (rv.random_01() < prob_on) /* transfer to off */
                        burst[i]->on = 0;
                } else { /* previous circle off */
                    if (rv.random_01() < prob_off) {/* transfer to on */
                        burst[i]->dest = traffic_entry_(i, rv);
                        burst[i]->on =  1;
                    }
                }
            } while (burst[i]->on != sbk);

            if (burst[i]->on) {/* if on, then generate traffic */
                A[i] = burst[i]->dest;
            }
        }
    }
};

class SwitchEvent {
public:
    /**\brief Package arrival event
     *
     * @param params
     * @param status
     * @param instruments
     * @param tgen
     */
    static void packet_arrival(lws_param_t &params, lws_status_t &status, lws_inst_t &instruments, TrafficGenerator *tgen, RV& rv) {
        int i, k;

        /* init A */
        for (i = 0;i < params.N;++ i) status.A[i] = LWS_NOARRIVAL;

        /* generate traffic */
        tgen->run(status.A, rv);

        /* arrival */
        for (i = 0;i < params.N;++ i){
            k = status.A[i];

            //std::cout << "(" << i << ", " << k << ")" << std::endl;
            if (k != LWS_NOARRIVAL){
                assert(k >= 0 && k < params.N);
                /* instruments update arrivals */
                ++ instruments.T_arr;
                if (params.type != LWS_THROUGHPUT) status.B[i][k].push(std::make_shared<packet_t>(i, k, status.cur_time));

                ++ status.Q[i][k];
            }
        }

        //std::cout << "packet arrival " << std::endl;
    }
    /**\brief Package departure
     *
     * @param params
     * @param status
     * @param instruments
     */
    static void packet_departure(lws_param_t &params, lws_status_t &status, lws_inst_t &instruments){
        int i, k, delay;

        for (i = 0;i < params.N;++ i){
            k = status.S[i];
            if (k >= 0 && k < params.N){/* valid schedule */
                if (status.Q[i][k] > 0){/* cells available */

                    if (params.type != LWS_THROUGHPUT){
                        //std::cout << "recording ..." << std::endl;
                        std::shared_ptr<packet_t> pkt = status.B[i][k].front(); /* gte info of head of line packet go */
                        assert(i == pkt->src && k == pkt->dest);
                        // std::cout << "pkt - time = " << pkt->time << ", ct = " << status.cur_time << std::endl;
                        // std::cout << status.B[i][k].size() << std::endl;
                        delay = status.cur_time - pkt->time;
                        status.B[i][k].pop(); /* let go hol packet */
                        if (delay > instruments.max_delay) instruments.max_delay = delay;
                        instruments.sum_delay += (double) delay;
                        instruments.mean_delay_count += 1.0;

                        instruments.sum_delay_per_voq[i][k] += (double) delay;
                        instruments.delay_counter_per_voq[i][k] += 1.0;

                        hdr_record_value(
                                instruments.histogram,
                                delay + 1); /* change delay to delay + 1, since 0 is not supported */
                    }



                    -- status.Q[i][k]; /* pay attention */


                    /* instrument update_delays, and departure */
                    ++ instruments.T_dep;

                }
            }
        }

        //std::cout << "packet depature " << std::endl;
    }
};



class UT{
public:
    static inline void identical_schedule(std::vector<int>& S){
        for (int i = 0;i < S.size();++ i) S[i] = i;
//        nlohmann::json j = {
//                {"schedule", S}
//        };
//        std::cout << j.dump(4) << std::endl;
    }
    static inline void rpermute(std::vector<int>& S, RV& rv) {

        identical_schedule(S);
        for (int i = S.size() - 1;i >= 0;-- i) std::swap(S[i], S[rv.random_int(i+1)]); /* duplicate random_shuffle */
    }
    /**
     * check whether S is a perfect matching
     * @param S
     * @return
     */
    static bool is_matching(std::vector<int>& S)
    {
        int len = S.size();
        std::vector<int> visited(S.size(), LWS_UNMATCHED);
        bool result = true;

        for(int i = 0;i < len;++ i)
            if (S[i] >= 0 && S[i] < len) visited[S[i]] = LWS_MATCHED;
            else { result = false; break; }

        for (int i = 0;i < len;++ i)
            if (visited[i] != LWS_MATCHED) { result = false; break; }
        return result;
    }
    /**
     * check whether S is a valid matching
     * @param S
     * @return
     */
    static bool is_partial_matching(std::vector<int>& S){
        int len = S.size();
        std::vector<int> visited(len, LWS_UNMATCHED);
        bool result = true;

        int k;
        for(int i = 0;i < len;++ i)
            if (S[i] >= 0 && S[i] < len) {/* valid */
                k = S[i];
                if (visited[k] != LWS_MATCHED) visited[k] = LWS_MATCHED;
                else { result = false; break; }
            }

        return result;
    }
    /**
     * make a valid matching perfect
     * @param S
     */
    static void fix_matching(std::vector<int>& S)
    {
        int len = S.size();
        std::vector<int> is_matched(len, LWS_UNMATCHED);
        std::vector<int> unmatched_in;
        std::vector<int> unmatched_out;

        int i, j, k;
        // int unmatched_counter = 0;

        /*! find all unmatched inputs & mark all matched inputs */
        for (i = 0;i < len;++ i)
            if (S[i] != LWS_UNMATCHED) is_matched[S[i]] = LWS_MATCHED; /*! mark matched output */
            else unmatched_in.push_back(i); /*! record unmatched inputs */

        /*! find all unmatched outputs */
        for (k = 0;k < len;++ k)
            if (is_matched[k] == LWS_UNMATCHED) unmatched_out.push_back(k);

        assert(unmatched_in.size() == unmatched_out.size());

        /*! matched all the unmatched inputs and outputs (round-robin) */
        for (j = 0;j < unmatched_in.size();++ j) {
            i = unmatched_in[j];
            k = unmatched_out[j];
            S[i] = k;
        }
    }
    template <class T>
    static int weighted_selection(std::vector<int>& items, std::vector<T>& weights, RV& rv, unsigned long len=0)
    {
        if (len == 0) len = items.size();
        assert(len <= weights.size());

        std::vector<T> cumsum(len, 0);

        int i, j = len - 1;
        double w;

        assert(weights[0] >= 0);
        cumsum[0] = weights[0];
        for (i = 1;i < len;++ i) {
            assert(weights[i] >= 0);
            cumsum[i] = cumsum[i - 1] + weights[i];
        }

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return items[j];
    }
    template <class T>
    static int weighted_selection(std::vector<T>& weights, RV& rv)
    {
        unsigned long len = weights.size();


        std::vector<T> cumsum(len, 0);

        int i, j = len - 1;
        double w;

        assert(weights[0] >= 0);
        cumsum[0] = weights[0];
        for (i = 1;i < len;++ i) {
            assert(weights[i] >= 0);
            cumsum[i] = cumsum[i - 1] + weights[i];
        }
        if (cumsum[len - 1] == 0) return -1;

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return j;
    }
    template <class T, class W>
    static int weighted_selection(std::vector<T>& weights, RV& rv, W* f)
    {
        assert(f->f(0) == 0 || f->f(0) == std::numeric_limits<double>::infinity());
        unsigned long len = weights.size();
        int i, j = len - 1, k;
        double w;

        if (f->f(0) == std::numeric_limits<double>::infinity()){/* infinity weight func */
            std::vector<int> rand_perm(len, 0);
            UT::rpermute(rand_perm, rv);

            w = 0;
            k = -1;
            for (std::vector<int>::iterator it=rand_perm.begin();it != rand_perm.end();++ it){
                i = *it;
               if (weights[i] > w){k = i; w = weights[i];}
            }
            return k;
        }


        std::vector<double> cumsum(len, 0);



        assert(f->f(weights[0]) >= 0);
        cumsum[0] = f->f(weights[0]);
        for (i = 1;i < len;++ i) {
            assert(f->f(weights[i]) >= 0);
            cumsum[i] = cumsum[i - 1] + f->f(weights[i]);
        }
        if (cumsum[len - 1] == 0) return -1;

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return j;
    }
    template <class T, class W>
    static int weighted_selection(std::vector<int> items, std::vector<T>& weights, RV& rv, W* f, unsigned long len = 0)
    {
        assert(f->f(0) == 0 || f->f(0) == std::numeric_limits<double>::infinity());
        if (len == 0) len = weights.size();
        assert(len <= items.size());

        int i, j = len - 1, k;
        double w;

        if (f->f(0) == std::numeric_limits<double>::infinity()){/* infinity weight func */
            std::vector<int> rand_perm(len, 0);
            UT::rpermute(rand_perm, rv);

            w = 0;
            k = -1;
            for (std::vector<int>::iterator it=rand_perm.begin();it != rand_perm.end();++ it){
                i = *it;
                if (weights[i] > w){k = i; w = weights[i];}
            }
            return k;
        }


        std::vector<double> cumsum(len, 0);



        assert(f->f(weights[0]) >= 0);
        cumsum[0] = f->f(weights[0]);
        for (i = 1;i < len;++ i) {
            assert(f->f(weights[i]) >= 0);
            cumsum[i] = cumsum[i - 1] + f->f(weights[i]);
        }
        if (cumsum[len - 1] == 0) return -1;

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return items[j];
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S, std::vector<std::vector<int> >& weights) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */
        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) break; /* back to i */
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            if (w < 0) selected_matching_id[c] = 1;
            ++ c;
        }

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) 
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (w < 0 && Ouroboros[cycle_length - 1] == 0) selected_matching_id[c] = 1;
            ++ c;
        }

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                     int& cn_t, int& cn_o) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (Ouroboros[cycle_length - 1] == 0) {
                selected_matching_id[c] = w < 0 ? 1:0;
                ++ cn_o;
            }
            ++ c;
        }
        cn_t += c;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros, int(*decisionFunc)(const int x, const int y)) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int leader;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            leader = i;/* leader init */

            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];

            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];

                leader = leader < current?leader:current;

                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (Ouroboros[cycle_length - 1] == 0) selected_matching_id[c] = w < 0?1:0;
            else
            {
                int wr = 0, wb = 0;
                int cur_node = leader;
                // total weight
//                for (int r = 0;r <= static_cast<int>(std::ceil(std::log2(N)));++ r){
                for (int r = 0;r < N;++ r){
                    wr += weights[cur_node][S1[cur_node]];
                    wb += weights[S2_reverse[S1[cur_node]]][S1[cur_node]];
                    cur_node = S2_reverse[S1[cur_node]];
                }
                selected_matching_id[c] = decisionFunc(wr, wb) > 0?0:1;
            }

            ++ c;
        }

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                      int(*decisionFunc)(const int x, const int y), int& cn_t, int& cn_o, int& cn_w) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int leader;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            leader = i;/* leader init */

            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];

            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];

                leader = leader < current?leader:current;

                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (Ouroboros[cycle_length - 1] == 0) {
                selected_matching_id[c] = w < 0?1:0;
                ++ cn_o;
            }
            else
            {
                int wr = 0, wb = 0;
                int cur_node = leader;
                // total weight
//                for (int r = 0;r <= static_cast<int>(std::ceil(std::log2(N)));++ r){
                for (int r = 0;r < N;++ r){
                    wr += weights[cur_node][S1[cur_node]];
                    wb += weights[S2_reverse[S1[cur_node]]][S1[cur_node]];
                    cur_node = S2_reverse[S1[cur_node]];
                }
                selected_matching_id[c] = decisionFunc(wr, wb) >= 0?0:1;

                cn_w += ((w < 0)?1:0) == selected_matching_id[c] ? 0:1;
            }

            ++ c;
        }
        cn_t += c;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S, std::vector<std::vector<int> >& weights, std::ostream& os) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */
        std::vector<int> cycle_length_recorder;

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) 
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;
            cycle_length_recorder.push_back(cycle_length);
            unmatched_inputs_pre = unmatched_inputs;
            if (w < 0) selected_matching_id[c] = 1;
            ++ c;
        }

        // record to stream
        os << c << " ";
        for (auto cl: cycle_length_recorder) os << cl << " ";
        os << std::endl;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
};

class Scheduler {
public:
    int port_num;
    Scheduler(int p): port_num(p) {}
    virtual void run(lws_param_t &g_params, lws_status_t &g_status) = 0;
    virtual void reset() = 0;
};


class iScheduler: virtual public Scheduler {
public:
    typedef std::vector<std::vector<int> > Matrix;
    std::vector<int> input_matched;
    std::vector<int> output_matched;
    std::vector<int> check;
    int rounds;

    iScheduler(int p): Scheduler(p), input_matched(p, 0), output_matched(p, 0), check(p, 0){
        rounds = static_cast<int>(std::ceil(std::log2(p)));
    }
    virtual void run(lws_param_t &g_params, lws_status_t &g_status) = 0;
    virtual void reset() = 0;
};


void single_run(LWS_SwitchCore& sw, RV &rv, Scheduler *sch, std::ostream& results){
    if (sw.params.model_option == LWS_ALL_MODELS) {std::cerr << "Can not run all models in single run" << std::endl; return;}
    if (sw.params.verbosity >= 1) sw.print_all();

    TrafficGenerator *tgen = NULL;

    if (sw.params.inject_option == LWS_ON_OFF_BURST)
        tgen = new BurstTrafficGenerator(sw.params.N, sw.params.load, sw.params.model_option,
                                         sw.params.average_burst_length); /* traffic generator */
    else
        tgen = new TrafficGenerator(sw.params.N, sw.params.load, sw.params.model_option);


    for (int ts = 0; ts < sw.params.MAX_M; ++ts) {
        /* set time slot */
        sw.status.cur_time = ts;

        SwitchEvent::packet_arrival(sw.params, sw.status, sw.instruments, tgen, rv);

        /* put scheduling here */
        sch->run(sw.params, sw.status);

        SwitchEvent::packet_departure(sw.params, sw.status, sw.instruments);
        /* put instruments printing here if necessary */
        if (sw.params.MAX_M - 1 == ts) sw.show(results);
        if ((ts > 0 && (ts % (sw.params.MAX_M / LWS_OUT_FREQ) == 0)) || ts == sw.params.MAX_M - 1) sw.show(std::cout,ts == sw.params.MAX_M - 1);
    }
    if (sw.params.verbosity >= 2)
        hdr_percentiles_print(
                sw.instruments.histogram,
                stdout,  // File to write to
                5,  // Granularity of printed values
                1.0,  // Multiplier for results
                CLASSIC);

    // print voq length
    std::cout << "===========================================\n";
    for (int i = 0;i < sw.params.N;++ i){
        for (int j = 0;j < sw.params.N;++ j){
            std::cout << ((sw.instruments.delay_counter_per_voq[i][j] > 0)?(sw.instruments.sum_delay_per_voq[i][j]/sw.instruments.delay_counter_per_voq[i][j]):-1);
            std::cout << " ";
        }
        std::cout << "\n";
    }
    std::cout << "===========================================\n";


    // running time printing
    std::chrono::high_resolution_clock::time_point stopTime = std::chrono::high_resolution_clock::now();

    auto duration_hours = std::chrono::duration_cast<std::chrono::hours>(stopTime - sw.instruments.start);
    stopTime -= duration_hours;
    auto duration_minutes = std::chrono::duration_cast<std::chrono::minutes>(stopTime - sw.instruments.start);
    stopTime -= duration_minutes;
    auto duration_seconds = std::chrono::duration_cast<std::chrono::seconds>(stopTime - sw.instruments.start);
    stopTime -= duration_seconds;
    auto duration_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - sw.instruments.start);
    std::cout << ">>>>>>> elapsed time: ";
    if (duration_hours.count() > 0) std::cout << duration_hours.count() << " hours ";
    if (duration_hours.count() + duration_minutes.count() > 0) std::cout << duration_minutes.count() << " minutes ";
    if (duration_hours.count() + duration_minutes.count() + duration_seconds.count() > 0) std::cout << duration_seconds.count() << "." << duration_milliseconds.count() << " seconds <<<<<\n";

    sw.reset();/* reset */
    sch->reset(); /* reset schedule */

    assert(tgen != NULL);
    delete tgen;
}

void main_run(LWS_SwitchCore& sw, RV& rv, Scheduler *sch, std::ostream& results){

    sw.show_header(results, false);

    if (sw.params.inject_option == LWS_ON_OFF_BURST) {/* burst traffic */
        for (auto load: sw.params.loads) {/* each load */
            for (auto bs: sw.params.abls) {/* each burst size */
                sw.params.load = load;
                sw.params.average_burst_length = bs;
                if (sw.params.model_option == LWS_ALL_MODELS) {/* all models */

                    for (int model = LWS_UNIFORM; model <= LWS_DIAGONAL; ++model) {
                        sw.params.model_option = model;
                        single_run(sw, rv, sch, results);
                    }

                    sw.params.model_option = LWS_ALL_MODELS;
                } else {
                    single_run(sw, rv, sch, results);
                }
            }
        }
    }else{
        for (auto load: sw.params.loads) {/* each load */
            sw.params.load = load;
            if (sw.params.model_option == LWS_ALL_MODELS) {
                for (int model = LWS_UNIFORM;model <= LWS_DIAGONAL;++ model) {
                    sw.params.model_option = model;
                    single_run(sw, rv, sch, results);
                }
                sw.params.model_option = LWS_ALL_MODELS;

            }else {
                single_run(sw, rv, sch, results);
            }
        }

    }
}

//void single_run(LWS_SwitchCore& sw, RV &rv, Scheduler *sch, std::ostream& results, std::ostream& stats){
//    if (sw.params.model_option == LWS_ALL_MODELS) {std::cerr << "Can not run all models in single run" << std::endl; return;}
//    if (sw.params.verbosity >= 1) sw.print_all();
//
//    TrafficGenerator *tgen = NULL;
//
//    if (sw.params.inject_option == LWS_ON_OFF_BURST)
//        tgen = new BurstTrafficGenerator(sw.params.N, sw.params.load, sw.params.model_option,
//                                         sw.params.average_burst_length); /* traffic generator */
//    else
//        tgen = new TrafficGenerator(sw.params.N, sw.params.load, sw.params.model_option);
//
//
//    for (int ts = 0; ts < sw.params.MAX_M; ++ts) {
//        /* set time slot */
//        sw.status.cur_time = ts;
//
//        SwitchEvent::packet_arrival(sw.params, sw.status, sw.instruments, tgen, rv);
//
//        /* put scheduling here */
//        sch->run(sw.params, sw.status);
//
//        SwitchEvent::packet_departure(sw.params, sw.status, sw.instruments);
//        /* put instruments printing here if necessary */
//        if (sw.params.MAX_M - 1 == ts) sw.show(results);
//        if ((ts > 0 && (ts % (sw.params.MAX_M / LWS_OUT_FREQ) == 0)) || ts == sw.params.MAX_M - 1) sw.show(std::cout,ts == sw.params.MAX_M - 1);
//    }
//    if (sw.params.verbosity >= 1)
//        hdr_percentiles_print(
//                sw.instruments.histogram,
//                stdout,  // File to write to
//                5,  // Granularity of printed values
//                1.0,  // Multiplier for results
//                CLASSIC);
//
//    stats << sw.params.load
//          << ","
//          << sw.params.model_option
//          << ","
//          <<
//
//    sw.reset();/* reset */
//    sch->reset(); /* reset schedule */
//
//    assert(tgen != NULL);
//    delete tgen;
//}
//
//void main_run(LWS_SwitchCore& sw, RV& rv, Scheduler *sch, std::ostream& results, std::ostream& stats){
//
//    sw.show_header(results, false);
//
//    if (sw.params.inject_option == LWS_ON_OFF_BURST) {/* burst traffic */
//        for (auto load: sw.params.loads) {/* each load */
//            for (auto bs: sw.params.abls) {/* each burst size */
//                sw.params.load = load;
//                sw.params.average_burst_length = bs;
//                if (sw.params.model_option == LWS_ALL_MODELS) {/* all models */
//
//                    for (int model = LWS_UNIFORM; model <= LWS_DIAGONAL; ++model) {
//                        sw.params.model_option = model;
//                        single_run(sw, rv, sch, results);
//                    }
//
//                    sw.params.model_option = LWS_ALL_MODELS;
//                } else {
//                    single_run(sw, rv, sch, results);
//                }
//            }
//        }
//    }else{
//        for (auto load: sw.params.loads) {/* each load */
//            sw.params.load = load;
//            if (sw.params.model_option == LWS_ALL_MODELS) {
//                for (int model = LWS_UNIFORM;model <= LWS_DIAGONAL;++ model) {
//                    sw.params.model_option = model;
//                    single_run(sw, rv, sch, results);
//                }
//                sw.params.model_option = LWS_ALL_MODELS;
//
//            }else {
//                single_run(sw, rv, sch, results);
//            }
//        }
//
//    }
//}
#endif //LWS_H
