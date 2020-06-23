#ifndef ILQF_SHAKEUP_HPP
#define ILQF_SHAKEUP_HPP

#include "../iLQF/iLQF.hpp"

/**
 * class for iLQF with Weighted Shakeup
 */
class iLQF_ShakeUp : public iLQF {
 public:
  iLQF_ShakeUp(int p, unsigned seed)
      : Scheduler(p), iScheduler(p), iLQF(p, seed) {}
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);
  virtual void reset() { iLQF::reset(); }
  void weighted_shakeup(lws_param_t &g_params, lws_status_t &g_status);
};

void iLQF_ShakeUp::run(lws_param_t &g_params, lws_status_t &g_status) {
  iLQF::run(g_params, g_status);
  weighted_shakeup(g_params, g_status);
}

/**
 * weighted shakeup
 * @param g_params
 * @param g_status
 */
void iLQF_ShakeUp::weighted_shakeup(lws_param_t &g_params,
                                    lws_status_t &g_status) {
  UT::rpermute(rand_perm, rv);
  int i, k, pre_i;
  for (std::vector<int>::iterator it = rand_perm.begin(); it != rand_perm.end();
       ++it) {
    i = *it;
    if (input_matched[i] == LWS_UNMATCHED) {         /* unmatched */
      k = UT::weighted_selection(g_status.Q[i], rv); /* weighted choose one */
      if (k == -1) continue;                         /* all VOQs are empty */
      if (output_matched[k] == LWS_UNMATCHED) {
        input_matched[i] = k;
        output_matched[k] = i;
        g_status.S[i] = k;
      } else {
        pre_i = output_matched[k];
        if (rv.random_01() < (double)(g_status.Q[i][k]) /
                                 (g_status.Q[i][k] + g_status.Q[pre_i][k])) {
          g_status.S[pre_i] = LWS_UNMATCHED;
          input_matched[pre_i] = LWS_UNMATCHED;
          input_matched[i] = k;
          output_matched[k] = i;
          g_status.S[i] = k;
        }
      }
    }
  }
  assert(UT::is_partial_matching(g_status.S));
}
#endif