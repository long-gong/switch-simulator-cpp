//
// iLQF
// Created by Long Gong on 10/1/16.
//
#ifndef ILQF_HPP
#define ILQF_HPP

#include "lws.hpp"

class iLQF : virtual public iScheduler {
 public:
  RV rv;
  std::vector<int> rand_perm;
  iLQF(int port_num, unsigned seed)
      : Scheduler(port_num),
        iScheduler(port_num),
        rv(seed),
        rand_perm(port_num, 0) {}
  virtual void run(lws_param_t& g_params, lws_status_t& g_status);
  virtual void reset();
  void one_round_(Matrix& VOQs, std::vector<int>& S);
};

void iLQF::one_round_(Matrix& VOQs, std::vector<int>& S) {
  int N = S.size();
  int i, j, k, c;

  /* Request & Grant Phase */
  for (k = 0; k < N; ++k) { /* for each output port */
    i = -1;
    check[k] = -1;
    if (output_matched[k] == LWS_UNMATCHED) { /* unmatched before */
      UT::rpermute(rand_perm, rv);
      for (j = 0; j < N; ++j) { /* for each input port in random order */
        c = rand_perm[j];
        /* pick the unmatched ingress with largest queue length */
        if (input_matched[c] == LWS_UNMATCHED && VOQs[c][k] > 0) {
          if (i == -1)
            i = c;
          else if (VOQs[c][k] > VOQs[i][k])
            i = c;
        }
      }
    }
    if (i != -1) check[k] = i;
  }
  /* Accept Phase */
  for (i = 0; i < N; ++i) { /* for each input port */
    k = -1;
    if (input_matched[i] == LWS_UNMATCHED) { /* unmatched before */
      UT::rpermute(rand_perm, rv);
      for (j = 0; j < N; ++j) { /* for each output port in random order */
        c = rand_perm[j];
        /* pick the unmatched egress with largest queue length */
        if (check[c] == i) {
          if (k == -1)
            k = c;
          else if (VOQs[i][c] > VOQs[i][k])
            k = c;
        }
      }
    }
    if (k != -1) {
      input_matched[i] = k;
      output_matched[k] = i;
      S[i] = k;
    }
  }
}
void iLQF::run(lws_param_t& g_params, lws_status_t& g_status) {
  int N = g_params.N;
  assert(N == port_num);
  int i, t;

  for (i = 0; i < N; ++i) { /* init parameters */
    g_status.S[i] = LWS_UNMATCHED;
    input_matched[i] = LWS_UNMATCHED;
    output_matched[i] = LWS_UNMATCHED;
  }

  for (t = 0; t < rounds + 1; ++t) one_round_(g_status.Q, g_status.S);
}

void iLQF::reset() {
  // Do nothing
}
#endif