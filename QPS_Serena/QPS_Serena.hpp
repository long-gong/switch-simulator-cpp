//
//
#ifndef QPS_SERENA_HPP
#define QPS_SERENA_HPP
#include "../QPS/QPS.hpp"
#include "../Serena/Serena.hpp"

class QPS_Serena : public QPS, public Serena {
 public:
  QPS_Serena(int port_num, unsigned seed, int b = 5, weight_fun *wf = NULL)
      : Scheduler(port_num),
        iScheduler(port_num),
        QPS(port_num, seed, b, wf),
        Serena(port_num) {}
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);
  virtual void reset() {
    QPS::reset();
    Serena::reset();
  }
};

void QPS_Serena::run(lws_param_t &g_params, lws_status_t &g_status) {
  if (internal_counter == 0) { /* initial */
    if (!UT::is_matching(g_status.S)) UT::identical_schedule(g_status.S);
  }

  std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
  std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);
  std::fill(r_matching.begin(), r_matching.end(), LWS_UNMATCHED);

  /*! Step 1: QPS */
  QPS::one_round_(g_status.Q, r_matching);

  UT::fix_matching(r_matching); /*! round-robin match unmatched in-out pairs */

  UT::merge(r_matching, g_status.S, merged_matching, g_status.Q);
  g_status.S.swap(merged_matching);

  ++internal_counter;
}
#endif