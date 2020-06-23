# SWITCH SIMULATOR 

[![Build Status](https://travis-ci.org/long-gong/switch-simulator.svg?branch=master)](https://travis-ci.org/long-gong/switch-simulator)

Simulation codes (full version), C++ version of [our old simulation codes in C](https://github.com/long-gong/switch-simulator) and adding more benchmark algorithms and more "features" (e.g., P95 delay), for our switching paper:

[Long Gong](https://lgong30.github.io/), Paul Tune, Liang Liu, Sen Yang, and [Jun (Jim) Xu](https://www.cc.gatech.edu/home/jx/). 2017. [Queue-Proportional Sampling: A Better Approach to Crossbar Scheduling for Input-Queued Switches](https://www.cc.gatech.edu/home/jx/reprints/Gong%20et%20al.%20-%202017%20-%20Queue-Proportional%20Sampling%20A%20Better%20Approach%20to%20.pdf). Proc. ACM Meas. Anal. Comput. Syst. 1, 1, Article 3 (June 2017), 33 pages. DOI:https://doi.org/10.1145/3084440
  
## Repository Structure

    .
    ├── common                   Source codes shared by different algorithms
    ├── FQPP_QPA_iSLIP           Source codes for variants of QPS-iSLIP (with PA)
    ├── FQPP_QPA_Serena          Source codes for variants of QPS-SERENA (with PA)
    ├── FQPS_iSLIP               Source codes for variants of QPS-iSLIP 
    ├── FQPS_Serena              Source codes for variants of QPS-SERENA 
    ├── iLQF                     Source codes for iLQF
    ├── iLQF_ShakeUp             Source codes for iLQF-ShakeUp
    ├── iSLIP                    Source codes for iSLIP
    ├── iSLIP_ShakeUp            Source codes for iSLIP-ShakeUp
    ├── MWM                      Source codes for Maximum Weighted Maximum (MWM)
    ├── O1                       Source codes for the O(1) algorithm
    ├── py                       Some useful python script
    ├── QPP_QPA                  Source codes for QPS (with PA)
    ├── QPP_QPA_iSLIP            Source codes for QPS-iSLIP (with PA)
    ├── QPP_QPA_Serena           Source codes for QPS-SERENA (with PA)
    ├── QPS                      Source codes for QPS 
    ├── QPS_iSLIP                Source codes for QPS-iSLIP 
    ├── QPS_Serena               Source codes for QPS-SERENA 
    ├── results                  Directory for storing simulation results
    └── Serena                   Source codes for SERENA 

## Benchmark Switching Algorithms (Selected)

Algorithms from literature:

1. **iSLIP** [[1](#1)]: the standard iSLIP algorithm,
2. **PIM** [[2](#2)]: parallel iterative matching,
3. **MWM** [[4](#4)]: maximum weighted matching,
4. **SERENA** [[5](#5)]: previous matching + arrival graph,
5. **iLQF** [[3](#3)]: iterative longest queue first, and
6. **ShakeUp** (including **iSLIP-ShakeUP** and **iLQF-ShakeUp**) [[7](#7)]: another "add-on" approach
7. **O(1)** [[8](#8)]: a O(1) crossbar scheduling algorithm based on Glauber dynamics 

Final proposals (**USED IN OUR PAPER**):

1. **QPS-iSLIP** [[6](#6)]: Queue Proportional Sampling (QPS) augmented iSLIP
2. **QPS-SERENA** [[6](#7)]: Queue Proportional Sampling (QPS) augmented SERENA

Other proposals (**COMPARED IN OUR PAPER**)

1. **FQPS-iSLIP** [[6](#6)]: Variants of Queue Proportional Sampling (QPS) augmented iSLIP
2. **FQPS-SERENA** [[6](#7)]: Variants of Queue Proportional Sampling (QPS) augmented SERENA
3. **QPS-iSLIP (with PA)**: Queue Proportional Sampling (QPS), with Proportional Accepting (PA), augmented iSLIP
4. **QPS-SERENA (with PA)**: Queue Proportional Sampling (QPS), with Proportional Accepting (PA), augmented SERENA

## Usage

+ Install dependencies: `./install_dependencies.sh`
+ compile: 
    ```bash
    mkdir build 
    cd build
    cmake ..
    make
    ```

## Traffic Modules

### Traffic Patterns

1. uniform,
2. diagonal,
3. log diagonal, and
4. quasi diagonal.

### Arrival Processes

1. Bernoulli,
2. Pareto bursts

## Acknowledgements

This project was modified from [Dr. Bill Lin](http://cwcserv.ucsd.edu/~billlin/)'s original source codes.

## Authors

+ Long Gong long.github@gmail.com

## References

[<a id="1">1</a>] [McKeown, N.](http://yuba.stanford.edu/~nickm/), 1999. [The iSLIP scheduling algorithm for input-queued switches](https://www.cs.rutgers.edu/~sn624/552-F18/papers/islip.pdf). IEEE/ACM transactions on networking, 7(2), pp.188-201.  
[<a id="2">2</a>] Anderson, T.E., Owicki, S.S., Saxe, J.B. and Thacker, C.P., 1993. [High-speed switch scheduling for local-area networks](https://dl.acm.org/doi/abs/10.1145/161541.161736). ACM Transactions on Computer Systems (TOCS), 11(4), pp.319-352.  
[<a id="3">3</a>] [McKeown, N.W.](http://yuba.stanford.edu/~nickm/), 1995. [Scheduling algorithms for input-queued cell switches](http://yuba.stanford.edu/~nickm/papers/nickMcKeown_thesis.pdf) (Doctoral dissertation, University of California, Berkeley).  
[<a id="4">4</a>] [McKeown, N., Mekkittikul](http://yuba.stanford.edu/~nickm/), A., Anantharam, V. and Walrand, J., 1999. [Achieving 100% throughput in an input-queued switch](http://yuba.stanford.edu/~nickm/papers/IEEE_COMM_V3.pdf). IEEE Transactions on Communications, 47(8), pp.1260-1267.  
[<a id="5">5</a>] Giaccone, P., Prabhakar, B. and [Shah, D.](https://devavrat.mit.edu/), 2003. [Randomized scheduling algorithms for high-aggregate bandwidth switches](https://ieeexplore.ieee.org/document/1197700). IEEE Journal on Selected Areas in Communications, 21(4), pp.546-559.
[<a id="6">6</a>] Gong, L., Tune, P., Liu, L., Yang, S. and Xu, J., 2017. [Queue-Proportional Sampling: A Better Approach to Crossbar Scheduling for Input-Queued Switches](https://www.cc.gatech.edu/home/jx/reprints/Gong%20et%20al.%20-%202017%20-%20Queue-Proportional%20Sampling%20A%20Better%20Approach%20to%20.pdf). Proceedings of the ACM on Measurement and Analysis of Computing Systems, 1(1), pp.1-33.  
[<a id="7">7</a>] Goudreau, M.W., Kolliopoulos, S.G. and Rao, S.B., 2000, March. [Scheduling algorithms for input-queued switches: randomized techniques and experimental evaluation](https://ieeexplore.ieee.org/document/832562). In Proceedings IEEE INFOCOM 2000. Conference on Computer Communications. Nineteenth Annual Joint Conference of the IEEE Computer and Communications Societies (Cat. No. 00CH37064) (Vol. 3, pp. 1634-1643). IEEE.  
[<a id="8">8</a>] Ye, S., Shen, Y. and Panwar, S., 2010, September. [An O (1) scheduling algorithm for variable-size packet switching systems](https://ieeexplore.ieee.org/document/5707119). In 2010 48th Annual Allerton Conference on Communication, Control, and Computing (Allerton) (pp. 1683-1690). IEEE.  

