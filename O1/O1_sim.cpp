#include "O1.hpp"

int main(int argc, char* argv[]) {
  LWS_SwitchCore sw(argv[0]);
  sw.init(argc, argv);
  // obtain a seed from the system clock:
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

  std::string fn = "../results/O1-" + std::to_string(sw.params.N) + "-" +
                   std::to_string(sw.params.MAX_M) + "-" +
                   std::to_string(seed1);
  if (sw.params.inject_option == LWS_ON_OFF_BURST) { /* burst */
    fn += "-burst";
  }
  fn += ".dat";

  std::ofstream results(fn, std::ofstream::out);

  New_Scheduler o1_s(sw.params.N); /* create a O1 scheduler */

  RV rv_for_tgen(sw.params.seed);

  main_run(sw, rv_for_tgen, &o1_s, results);

  sw.destroy();    /* pay attention */
  results.close(); /* close results file */

  return 0;
}