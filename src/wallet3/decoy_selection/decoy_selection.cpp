#include "decoy_selection.hpp"

namespace wallet
{
  std::vector<Decoy>
  DecoySelector::operator()(const Output& selected_output)
  {
    std::vector<Decoy> return_decoys;
    const size_t n_decoys = 13;

    // Select some random outputs according to gamma distribution
    std::random_device rd;
    std::default_random_engine rng(rd());

    // TODO(sean): these should be built using the distribution
    int64_t min_output_index = 100;
    int64_t max_output_index = 100000;
    constexpr int ALPHA = 1;
    constexpr int BETA = 2;
    std::gamma_distribution<double> distribution(ALPHA, BETA);

    // Build a distribution and apply a score to each element of available outputs depending
    // on distance from the number chosen. Lower score is better.
    std::vector<int64_t> decoy_indexes;
    for (size_t i = 0; i < n_decoys; ++i)
    {
      int64_t output_height_from_distribution = max_output_index - std::round(distribution(rng) * (max_output_index - min_output_index)/10);
      decoy_indexes.push_back(output_height_from_distribution);
    }

    // TODO(sean): we need to also request the chosen output
    auto decoy_promise = daemon->fetch_decoys(decoy_indexes);
    decoy_promise.wait();
    return decoy_promise.get();
  }
}  // namespace wallet
