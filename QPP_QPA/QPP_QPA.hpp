//! @brief   QPP_QPA Class
//!
//! Class for Queue-Proportional Proposing and
//! Queue-Proportional Accepting (and some assistant
//! classes). More details can be found in our QPS
//! paper.
#ifndef QPP_QPA_HPP
#define QPP_QPA_HPP

#include "lws.hpp"

/**
 * @brief      Class for weight_fun.
 *
 * This class provide interfaces to implementing
 * various (queue-length) function based queue
 * proportional proposing (QPS) schemes.
 */
class weight_fun {
 public:
  std::string name;
  weight_fun(const std::string &n) : name(n) {}
  virtual double f(int n) = 0;
  virtual ~weight_fun() {}
};

/**
 * @brief      "power" based weight function class
 *
 * This class implementing the interface weight_fun
 * with a set of power function, e.g., $n^2$ (n square)
 */
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

/**
 * @brief      QPP_QPA scheduler class
 *
 * This class implements Queue-Proportional Proposing
 * and Queue-Proportional Accepting scheduling policy.
 * The motivation of introducing it is to somehow
 * "solve" some fairness issue of QPS. More discussion
 * on this, please refer to our QPS paper.
 */
class QPP_QPA : virtual public iScheduler {
 public:
  // buffer size holding proposals requests at each output port
  int buffer_size;
  // vector for storing a random permutation
  std::vector<int> rand_perm;
  // The counters for storing proposal requests to each output port
  std::vector<int> req_counters;
  /**
   * @brief      The (proposal) request candidates
   *
   * This 2d vector stores (accepting) candidates at each
   * output port. More specifically, req_candidates[i]
   * stores all the proposal requests received by output
   * port (i+1) before its buffer gets full. Note that,
   * the arrival of request proposals are assumed to
   * be uniformly random.
   */
  std::vector<std::vector<int> > req_candidates;
  /**
   * @brief     The "queue length" associated with each candidate
   *
   * This 2d vector stores the "queue length" associated with
   * each candidate.
   */
  std::vector<std::vector<int> > req_candidate_qls;
  // random generator
  RV rv;
  /**
   * @brief        The weight function
   *
   * Note that, the default value of f is NULL,
   * which corresponds to QPS. Of course you can
   * also use f=n to trigger QPS. However, the
   * former one will call the optimized version
   * of QPS, which has O(1) complexity, while
   * the later has a complexity of O(N).
   */
  weight_fun *f;

  /**
   * @brief      Constructor
   *
   * @param[in]  port_num  The port number
   * @param[in]  seed      The seed
   * @param[in]  b         The buffer size
   * @param      wf        The pointer points to a weight function (optional,
   * default: null==>QPS, otherwise==>FQPS)
   */
  QPP_QPA(int port_num, unsigned seed, int b = 5, weight_fun *wf = NULL)
      : Scheduler(port_num),
        iScheduler(port_num),
        rv(seed),
        buffer_size(b),
        rand_perm(port_num, 0),
        req_counters(port_num, 0),
        req_candidates(port_num),
        req_candidate_qls(port_num) {
    f = wf;
    for (int i = 0; i < port_num; ++i) {
      req_candidates[i].resize(buffer_size, -1);
      req_candidate_qls[i].resize(buffer_size, 0);
    }
  }
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);
  virtual void reset();
  void one_round_(Matrix &VOQs, std::vector<int> &S);
};

void QPP_QPA::run(lws_param_t &g_params, lws_status_t &g_status) {
  int N = g_params.N;
  std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
  std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);

  for (int i = 0; i < N; ++i) g_status.S[i] = LWS_UNMATCHED;

  for (int t = 0; t < rounds + 1; ++t) one_round_(g_status.Q, g_status.S);
}

/**
 * @brief      One round for QPP_QPA
 *
 * This function runs one round of queue-proportional
 * proposing and queue-proportional accepting. More
 * details of this function can be found in our QPS
 * paper.
 *
 * TODO: This function can be optimized (in terms of time complexity)
 *
 * @param      VOQs  The (queue) lengths of all VOQs
 * @param      S     The schedule used in previous time slot
 */
void QPP_QPA::one_round_(Matrix &VOQs, std::vector<int> &S) {
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
        k = UT::weighted_selection<int>(VOQs[i], rv); /* call qps */
      else
        k = UT::weighted_selection<int, weight_fun>(VOQs[i], rv,
                                                    f); /* call fqps */

      if (k != -1) { /* not empty */
        ++req_counters[k];
        if (output_matched[k] == LWS_UNMATCHED) { /* unmatched before */
          if (req_counters[k] <=
              buffer_size) { /* have requests and less than buffer size */
            // record the input port and associated VOQ length
            req_candidates[k][req_counters[k] - 1] = i;
            req_candidate_qls[k][req_counters[k] - 1] = VOQs[i][k];
          }
        }
      }
    }
  }

  /* accept */
  for (k = 0; k < N; ++k) {
    if (req_counters[k] == 0)
      continue;  // no request proposal
    else {       // proportionally select a proposal
      if (f == NULL)
        i = UT::weighted_selection<int>(req_candidates[k], req_candidate_qls[k],
                                        rv, req_counters[k]);
      else
        i = UT::weighted_selection<int, weight_fun>(
            req_candidates[k], req_candidate_qls[k], rv, f, req_counters[k]);
      if (i == -1) i = LWS_UNMATCHED;
    }
    if (i != LWS_UNMATCHED && input_matched[i] == LWS_UNMATCHED) {
      input_matched[i] = k;
      output_matched[k] = i;
      S[i] = k;
    }
  }
}

void QPP_QPA::reset() {
  // do nothing
}
#endif