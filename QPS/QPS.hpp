//!@brief   Class for QPS
//!
//! This file implements Queue Proportional
//! Sampling (QPS) schemes
//!
#ifndef QPS_HPP
#define QPS_HPP

#include "lws.hpp"

class weight_fun {
 public:
  std::string name;
  weight_fun(const std::string &n) : name(n) {}
  virtual double f(int n) = 0;
  virtual ~weight_fun() {}
};

class pown : public weight_fun {
 public:
  int p;
  pown(int p) : weight_fun(""), p(p) {
    if (p == -1) {
      name = "n^{inf}";
    } else if (p == 0) {
      name = "log(n+1)";
    } else if (p > 0) {
      name += "n^" + std::to_string(p);
    } else {
      std::cerr << "p = " << p << " is not supported " << std::endl;
    }
  }
  virtual double f(int n) {
    if (p == -1) return std::numeric_limits<double>::infinity();
    if (p == 0) return std::log2(n + 1);
    return std::pow(n, p);
  }
};

class QPS : virtual public iScheduler {
 public:
  int buffer_size;
  std::vector<int> rand_perm;
  std::vector<int> req_counters;
  RV rv;
  weight_fun *f;

  QPS(int port_num, unsigned seed, int b = 5, weight_fun *wf = NULL)
      : Scheduler(port_num),
        iScheduler(port_num),
        rv(seed),
        buffer_size(b),
        rand_perm(port_num, 0),
        req_counters(port_num, 0) {
    f = wf;
  }
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);
  virtual void reset();
  void one_round_(Matrix &VOQs, std::vector<int> &S);
};

void QPS::run(lws_param_t &g_params, lws_status_t &g_status) {
  int N = g_params.N;
  std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
  std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);

  for (int i = 0; i < N; ++i) g_status.S[i] = LWS_UNMATCHED;

  for (int t = 0; t < rounds + 1; ++t) one_round_(g_status.Q, g_status.S);
}

/**
 * QPS one round
 * @param VOQs
 * @param S
 * @param input_matched
 * @param output_matched
 */
void QPS::one_round_(Matrix &VOQs, std::vector<int> &S) {
  int N = S.size();
  assert(N == port_num);

  int i, k, pre_i;

  std::fill(req_counters.begin(), req_counters.end(), 0); /* fill as 0 */
  std::fill(check.begin(), check.end(),
            LWS_UNMATCHED); /* fill as unmatched ?forgot */

  /* proposing & grant */
  UT::rpermute(rand_perm, rv); /* make sure every k inputs have the same prob */
  for (auto val : rand_perm) {
    i = val;
    if (input_matched[i] == LWS_UNMATCHED) { /* unmatched before */
      if (f == NULL)
        k = UT::weighted_selection<int>(VOQs[i], rv); /* qps */
      else
        k = UT::weighted_selection<int, weight_fun>(VOQs[i], rv, f);
      if (k != -1) {
        ++req_counters[k];
        if (output_matched[k] == LWS_UNMATCHED) { /* unmatched before */
          if (check[k] == LWS_UNMATCHED)
            check[k] = i; /* no request before */
          else if (req_counters[k] <=
                   buffer_size) { /* have requests and less than buffer size */
            pre_i = check[k];
            if (VOQs[i][k] > VOQs[pre_i][k]) check[k] = i;
          }
        }
      }
    }
  }

  /* accept */
  for (k = 0; k < N; ++k) {
    i = check[k];
    if (i != LWS_UNMATCHED && input_matched[i] == LWS_UNMATCHED) {
      input_matched[i] = k;
      output_matched[k] = i;
      S[i] = k;
    }
  }
}

void QPS::reset() {
  // do nothing
}
#endif