//
// Created by Long Gong on 1/12/17.
//

#ifndef LWS_NEW_HPP
#define LWS_NEW_HPP

#include "lws.hpp"

class New_Scheduler : virtual public Scheduler {
 public:
  unsigned seed;
  RV r;
  int internal_counter;
  std::vector<int> reverse_S;

  New_Scheduler(int p,
                unsigned seed =
                    std::chrono::system_clock::now().time_since_epoch().count())
      : Scheduler(p),
        seed(seed),
        r(seed),
        internal_counter(0),
        reverse_S(p){

        };

  virtual void run(lws_param_t &g_params, lws_status_t &g_status);

  virtual void reset() { internal_counter = 0; }
};

/**
 * @brief      O(1) randomized scheduler
 *
 * This function implements the randomized scheduling algorithm
 * proposed in "An O(1) scheduling algorithm for variable-size
 * packets", However, it does some simplification (only implementing
 * the part works for fixed-size).
 *
 * @param      g_params  The g parameters
 * @param      g_status  The g status
 */
void New_Scheduler::run(lws_param_t &g_params, lws_status_t &g_status) {
  int i, j;

  if (internal_counter == 0) {
    // initially using identical matching
    // UT::identical_schedule(g_status.S);
    // UT::identical_schedule(reverse_S);
    // CHANGE: changed to empty initialization
    std::fill(g_status.S.begin(), g_status.S.end(), -1);
    std::fill(reverse_S.begin(), reverse_S.end(), -1);
  }

  // generate H(n)
  i = r.random_int(g_params.N);
  j = r.random_int(g_params.N);

  double p;

  if (g_status.Q[i][j] == 0)  // return; // nothing to be done
  {                           // CHANGE: from return to setting p = 0
    p = 0;
  } else {  // calculate the probability
    p = std::log(g_status.Q[i][j]);
    p = p / (p + 1);
  }

  if (g_status.S[i] == j || (g_status.S[i] == -1 && reverse_S[j] == -1)) {
    if (r.random_01() <= p) {
      g_status.S[i] = j;
      reverse_S[j] = i;
    } else {  // S_{ij}(n) = 0
      g_status.S[i] = -1;
      reverse_S[j] = -1;
    }
  }

  ++internal_counter;  // CHANGE: added
}

#endif  // LWS_NEW_HPP