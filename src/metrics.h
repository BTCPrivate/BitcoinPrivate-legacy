// Copyright (c) 2016 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uint256.h"

#include <atomic>
#include <mutex>
#include <string>

struct AtomicCounter {
    std::atomic<uint64_t> value;

    AtomicCounter() : value {0} { }

    void increment(){
        ++value;
    }

    void decrement(){
        --value;
    }

    int get() const {
        return value.load();
    }
};

class AtomicTimer {
private:
    std::mutex mtx;
    uint64_t threads;
    int64_t start_time;
    int64_t total_time;

public:
    AtomicTimer() : threads(0), start_time(0), total_time(0) {}

    /**
     * Starts timing on first call, and counts the number of calls.
     */
    void start();

    /**
     * Counts number of calls, and stops timing after it has been called as
     * many times as start().
     */
    void stop();

    bool running();

    uint64_t threadCount();

    double rate(const AtomicCounter& count);
};

extern AtomicCounter transactionsValidated;
extern AtomicCounter ehSolverRuns;
extern AtomicCounter solutionTargetChecks;
extern AtomicTimer miningTimer;

void TrackMinedBlock(uint256 hash);

void MarkStartTime();
double GetLocalSolPS();

void TriggerRefresh();

void ConnectMetricsScreen();
void ThreadShowMetricsScreen();

/* https://github.com/hit9/img2txt */

const std::string METRICS_ART =
std::string("") +
"                            #%%%%%%%%%%%%%%%%%%%%%(.                            \n"+
"                       *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#.                      \n"+
"                   .%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#                   \n"+
"                .%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#                \n"+
"              (%%%%%%%%%%%%%%%%%%%%%%%%%,(%%%%%%%%%%%%%%%%%%%%%%%%.             \n"+
"            #%%%%%%%%%%%%%%%%%%%%%%%%%%*     %%%%%%%%%%%%%%%%%%%%%%%*           \n"+
"          #%%%%%%%%%%%%%%%%%%%%%%%%%%%%     ,%%%%      %%%%%%%%%%%%%%%.         \n"+
"        ,%%%%%%%%%%%%%%%%%%*#%%%%%%%%%*     %%%%(     #%%%%%%%%%%%%%%%%%        \n"+
"       #%%%%%%%%%%%%%%%%%%(        *#%     .%%%%      %%%%%%%%%%%%%%%%%%%.      \n"+
"      %%%%%%%%%%%%%%%%%%%%                  *#%(     #%%%%%%%%%%%%%%%%%%%%/     \n"+
"     %%%%%%%%%%%%%%%%%%%%%#/.                        %%%%%%%%%%%%%%%%%%%%%%(    \n"+
"    %%%%%%%%%%%%%%%%%%%%%%%%%%%#                        .#%%%%%%%%%%%%%%%%%%(   \n"+
"   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%          ,                 %%%%%%%%%%%%%%%%%,  \n"+
"  /%%%%%%%%%%%%%%%%%%%%%%%%%%%%*         #%%%%%%(             %%%%%%%%%%%%%%%%  \n"+
"  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%          %%%%%%%%%%#           %%%%%%%%%%%%%%%# \n"+
" (%%%%%%%%%%%%%%%%%%%%%%%%%%%%*         #%%%%%%%%%%%%          /%%%%%%%%%%%%%%% \n"+
" %%%%%%%%%%%%%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%          /%%%%%%%%%%%%%%%/\n"+
" %%%%%%%%%%%%%%%%%%%%%%%%%%%%*         #%%%%%%%%%%%%           %%%%%%%%%%%%%%%%#\n"+
".%%%%%%%%%%%%%%%%%%%%%%%%%%%%               .,**,             #%%%%%%%%%%%%%%%%%\n"+
",%%%%%%%%%%%%%%%%%%%%%%%%%%%*                                %%%%%%%%%%%%%%%%%%%\n"+
".%%%%%%%%%%%%%%%%%%%%%%%%%%%          ,                  /%%%%%%%%%%%%%%%%%%%%%%\n"+
" %%%%%%%%%%%%%%%%%%%%%%%%%%*         #%%%%%%%%%%%%%%%%%%%#**%%%%%%%%%%%%%%%%%%%#\n"+
" %%%%%%%%%%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%*\n"+
" (%%%%%%%%%%%%%%%%%%%%%%%%*         #%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%% \n"+
"  %%%%%%%%%%%%%%%%%#*#%%%%          %%%%%%%%%%%%%%%,          #%%%%%%%%%%%%%%%# \n"+
"  /%%%%%%%%%%%%%%%%                #%%%%%%%%%%%%%%%           #%%%%%%%%%%%%%%%  \n"+
"   %%%%%%%%%%%%%%%                 %%%%%%%%%%%%%%(            %%%%%%%%%%%%%%%,  \n"+
"    %%%%%%%%%%%%%/.               #%%%(                      %%%%%%%%%%%%%%%(   \n"+
"     %%%%%%%%%%%%%%%%%%%#*       .%%%%                      %%%%%%%%%%%%%%%(    \n"+
"      %%%%%%%%%%%%%%%%%%%%%/     %%%%#                    ,%%%%%%%%%%%%%%%/     \n"+
"       #%%%%%%%%%%%%%%%%%%%     .%%%%                  ,%%%%%%%%%%%%%%%%%.      \n"+
"        ,%%%%%%%%%%%%%%%%%/     %%%%#     #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        \n"+
"          #%%%%%%%%%%%%%%%     .%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%.         \n"+
"            %%%%%%%%%%%%%%%(.  %%%%#     #%%%%%%%%%%%%%%%%%%%%%%%%%%*           \n"+
"              #%%%%%%%%%%%%%%%%%%%%(.    %%%%%%%%%%%%%%%%%%%%%%%%%.             \n"+
"                .%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#                \n"+
"                   ,%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#                   \n"+
"                       /%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#.                      \n"+
"                            *#%%%%%%%%%%%%%%%%%%%%%(.                           ";

