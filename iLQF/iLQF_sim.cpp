#include "iLQF.hpp"

int main(int argc, char* argv[]) {
  LWS_SwitchCore sw(argv[0]);
  sw.init(argc, argv);
  // obtain a seed from the system clock:
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

  std::string fn = "../results/iLQF-" + std::to_string(sw.params.N) + "-" +
                   std::to_string(sw.params.MAX_M) + "-" +
                   std::to_string(seed1);
  if (sw.params.inject_option == LWS_ON_OFF_BURST) { /* burst */
    fn += "-burst";
  }
  fn += ".dat";

  std::ofstream results(fn, std::ofstream::out);

  iLQF ilqf_s(sw.params.N, seed1); /* create a iLQF scheduler */

  RV rv_for_tgen(sw.params.seed);

  main_run(sw, rv_for_tgen, &ilqf_s, results);

  sw.destroy();    /* pay attention */
  results.close(); /* close results file */

  return 0;
}