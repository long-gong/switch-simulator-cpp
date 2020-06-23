#ifndef QPP_QPA_ISLIP_HPP
#define QPP_QPA_ISLIP_HPP

#include "../QPP_QPA/QPP_QPA.hpp"
#include "../iSLIP/iSLIP.hpp"

class QPP_QPA_iSLIP : public QPP_QPA, public iSLIP {
 public:
  QPP_QPA_iSLIP(int p, unsigned seed, int b = 5, weight_fun *f = NULL)
      : Scheduler(p), iScheduler(p), QPP_QPA(p, seed, b, f), iSLIP(p) {}
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);

  virtual void reset() {
    QPP_QPA::reset();
    iSLIP::reset();
  }
};

void QPP_QPA_iSLIP::run(lws_param_t &g_params, lws_status_t &g_status) {
  /* init input and out matched */
  std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
  std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);

  for (int i = 0; i < g_status.S.size(); ++i) g_status.S[i] = LWS_UNMATCHED;

  QPP_QPA::one_round_(g_status.Q, g_status.S);

  for (int t = 0; t < rounds; ++t)
    iSLIP::one_round_(g_status.Q, g_status.S, t == 0);
}

#endif