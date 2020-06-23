//
// Maximum Weighted Matching
//
// Created by Long Gong on 9/24/16.
//
#ifndef LWS_MWM_HPP
#define LWS_MWM_HPP

//#include <iostream>
//#include <vector>
#include <lemon/list_graph.h>
#include <lemon/matching.h>

#include "lws.hpp"

#define MWM_NOEDGE 0
#define MWM_NOMATE -1
#define MWM_INVALIDEDGEID -1
using lemon::INVALID;
/**\brief a class for MWM
 *
 */
class MWM : virtual public Scheduler {
 public:
  typedef lemon::ListGraph Graph;
  typedef Graph::Node Node;
  typedef Graph::Edge Edge;
  typedef Graph::EdgeMap<int> WeightMap;
  typedef lemon::MaxWeightedMatching<Graph, WeightMap> MaxWeightedMatching;
  typedef std::vector<std::vector<int> > Matrix;

  Graph g;

  std::vector<Node> node_vec;
  Matrix edge_id;

  MWM(int n) : Scheduler(n) {
    int tn = 2 * n;  // total number of nodes

    // add node to graph and record them
    for (int i = 0; i < tn; ++i) node_vec.push_back(g.addNode());

    // weights = new WeightMap(g);

    // initialize edge id
    edge_id.resize(n);
    for (int i = 0; i < n; ++i) {
      edge_id[i].resize(n);
      for (int j = 0; j < n; ++j) edge_id[i][j] = MWM_INVALIDEDGEID;
    }
  }
  virtual void run(lws_param_t &g_params, lws_status_t &g_status);
  virtual void reset() {
    // Do Nothing
  }
};

void MWM::run(lws_param_t &g_params, lws_status_t &g_status) {
  /* edit graph */
  assert(edge_id.size() == g_params.N);

  int i, j, k, eId;
  Node u, v;
  Edge e;

  WeightMap weights(g); /* create a map on g */

  for (i = 0; i < edge_id.size(); ++i) {
    for (k = 0; k < edge_id[0].size(); ++k) {
      j = g_params.N + k;
      u = node_vec[i];                /* input port */
      v = node_vec[j];                /* output port */
      eId = edge_id[i][k];            /* edge id */
      if (eId == MWM_INVALIDEDGEID) { /* no edge before */
        if (g_status.Q[i][k] > 0) {   /* has edge now */
          e = g.addEdge(u, v);
          weights[e] = g_status.Q[i][k];
          edge_id[i][k] = g.id(e);
        }
      } else { /* has edge before */
        e = g.edgeFromId(eId);
        if (g_status.Q[i][k] > 0) { /* still has edge */
          weights[e] = g_status.Q[i][k];
        } else {      /* no edge now */
          g.erase(e); /* remove edge */
          edge_id[i][k] = MWM_INVALIDEDGEID;
        }
      }
    }
  }

  MaxWeightedMatching mwm(g, weights); /* initialize an instance of mwm */
  mwm.run();                           /* run mwm */

  /* read results */
  for (i = 0; i < g_params.N; ++i) {
    u = node_vec[i];
    v = mwm.mate(u);
    if (v == INVALID)
      g_status.S[i] = MWM_NOMATE; /* make sure every i is setting */
    else {
      assert(g.id(v) >= g_params.N);
      g_status.S[i] = g.id(v) - g_params.N;
    }
  }

  //    return mwm.matchingWeight();
}
#endif