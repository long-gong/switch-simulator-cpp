#include "../QPP_QPA_iSLIP/QPP_QPA_iSLIP.hpp"

int main(int argc, char *argv[]) {
  LWS_SwitchCore sw(argv[0]);
  sw.init(argc, argv);
  // obtain a seed from the system clock:
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

  std::string fn = "../results/FQPP_QPA_iSLIP-" + std::to_string(sw.params.N) +
                   "-" + std::to_string(sw.params.MAX_M) + "-" +
                   std::to_string(seed1);
  if (sw.params.inject_option == LWS_ON_OFF_BURST) { /* burst */
    fn += "-burst";
  }
  fn += ".dat";

  std::ofstream results(fn, std::ofstream::out);

  weight_fun *f;
  RV rv_for_tgen(sw.params.seed);

  for (int i = -1; i <= 6; ++i) {
    f = new pown(i);
    results << "results for " << f->name << std::endl;
    std::cout << f->name << std::endl;
    QPP_QPA_iSLIP qpp_qpa_islip_s(sw.params.N, seed1, 5,
                                  f); /* create a FQPS_iSLIP scheduler */
    main_run(sw, rv_for_tgen, &qpp_qpa_islip_s, results);
    delete f;
  }

  sw.destroy();    /* pay attention */
  results.close(); /* close results file */

  return 0;
}