#include "QPS_Serena.hpp"

int main(int argc, char* argv[]) {
  LWS_SwitchCore sw(argv[0]);
  sw.init(argc, argv);
  // obtain a seed from the system clock:
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

  std::string fn = "../results/QPS_Serena-" + std::to_string(sw.params.N) +
                   "-" + std::to_string(sw.params.MAX_M) + "-" +
                   std::to_string(seed1);
  if (sw.params.inject_option == LWS_ON_OFF_BURST) { /* burst */
    fn += "-burst";
  }
  fn += ".dat";

  std::ofstream results(fn, std::ofstream::out);

  QPS_Serena qps_serena_s(
      sw.params.N, seed1,
      sw.params.accept_buffer); /* create a QPS-Serena scheduler */

  RV rv_for_tgen(sw.params.seed);

  main_run(sw, rv_for_tgen, &qps_serena_s, results);

  sw.destroy();    /* pay attention */
  results.close(); /* close results file */

  return 0;
}