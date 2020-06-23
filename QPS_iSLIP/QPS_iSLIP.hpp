#ifndef QPS_ISLIP_HPP
#define QPS_ISLIP_HPP

#include "../QPS/QPS.hpp"
#include "../iSLIP/iSLIP.hpp"

class QPS_iSLIP : public QPS, public iSLIP {
 public:
  QPS_iSLIP(int p, unsigned seed, int b = 5, weight_fun *f = NULL)
      : Scheduler(p), iScheduler(p), QPS(p, seed, b, f), iSLIP(p) {}
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);

  virtual void reset() {
    QPS::reset();
    iSLIP::reset();
  }
};

void QPS_iSLIP::run(lws_param_t &g_params, lws_status_t &g_status) {
  /* init input and out matched */
  std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
  std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);

  for (int i = 0; i < g_status.S.size(); ++i) g_status.S[i] = LWS_UNMATCHED;

  QPS::one_round_(g_status.Q, g_status.S);

  for (int t = 0; t < rounds; ++t)
    iSLIP::one_round_(g_status.Q, g_status.S, t == 0);
}

#endif