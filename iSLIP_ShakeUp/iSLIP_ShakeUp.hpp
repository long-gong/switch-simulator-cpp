#ifndef ISLIP_SHAKEUP_HPP
#define ISLIP_SHAKEUP_HPP

#include "../iSLIP/iSLIP.hpp"

/**
 * revised to the orthogonal version
 * as stated in the paper;
 * Note we only update the pointers
 * at the first iteration of iSLIP
 */
class iSLIP_ShakeUp : public iSLIP {
 public:
  std::vector<int> shakeup_cand;
  std::vector<int> shakeup_req;
  RV rv;
  iSLIP_ShakeUp(int p, unsigned seed, int round = 0)
      : Scheduler(p),
        iScheduler(p),
        iSLIP(p),
        shakeup_cand(p, 0),
        shakeup_req(p, 0),
        rv(seed) {
    if (round > 0) rounds = round; /* add round setting */
  }
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);
  virtual void reset() { iSLIP::reset(); }
  void shakeup(lws_param_t &g_params, lws_status_t &g_status);
};

void iSLIP_ShakeUp::shakeup(lws_param_t &g_params, lws_status_t &g_status) {
  int N = g_params.N;
  assert(N == port_num);
  int shakeup_counter = 0;
  int i, k, x, pre_i;
  /* shakeup request phase */
  for (i = 0; i < N; ++i) {
    if (input_matched[i] == LWS_UNMATCHED) { /* i is unmatched */
      shakeup_counter = 0;                   /* reset counter */
      for (k = 0; k < N; ++k) {
        if (g_status.Q[i][k] > 0) { /* queue not empty */
          shakeup_cand[shakeup_counter] = k;
          ++shakeup_counter;
        }
      }
      if (shakeup_counter > 0) { /* shakeup for i*/
        x = rv.random_int(shakeup_counter);
        shakeup_req[i] = shakeup_cand[x]; /* record */
      } else {
        shakeup_req[i] = -1;
      }
    } else {
      shakeup_req[i] = -1;
    }
  }
  /* shakeup grant phase */
  for (k = 0; k < N; ++k) {
    shakeup_counter = 0; /* reset counter */
    for (i = 0; i < N; ++i) {
      if (shakeup_req[i] == k) { /* has request(s)*/
        shakeup_cand[shakeup_counter] = i;
        ++shakeup_counter;
      }
    }
    if (shakeup_counter > 0) { /* has requests */
      x = rv.random_int(shakeup_counter);
      i = shakeup_cand[x];
      pre_i = output_matched[k];
      if (pre_i != -1) { /* remove previous matching */
        assert(g_status.S[pre_i] == k);
        assert(input_matched[pre_i] == k);
        g_status.S[pre_i] = LWS_UNMATCHED;
        input_matched[pre_i] = LWS_UNMATCHED;
      }
      assert(input_matched[i] == LWS_UNMATCHED);
      g_status.S[i] = k;
      output_matched[k] = i;
      input_matched[i] = k;
    }
  }
  assert(UT::is_partial_matching(g_status.S));
}

void iSLIP_ShakeUp::run(lws_param_t &g_params, lws_status_t &g_status) {
  std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
  std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);

  for (int i = 0; i < g_status.S.size(); ++i) g_status.S[i] = LWS_UNMATCHED;

  if (g_params.verbosity >= 3) std::cout << "rounds = " << rounds << std::endl;
  for (int t = 0; t < std::ceil((rounds + 1) * 1.0 / 2); ++t) {
    iSLIP::one_round_(g_status.Q, g_status.S, t == 0);
    shakeup(g_params, g_status);
  }
}

#endif
