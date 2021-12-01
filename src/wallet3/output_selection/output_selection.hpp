#pragma once

#include <vector>
#include "../output.hpp"

namespace wallet
{
  // OutputSelector will choose some a subset of outputs from the provided list of outputs according
  // to the output selection algorithm. The sum of the amounts in the returned outputs will be
  // greater than the amount passed as the second parameter.

  class OutputSelector
  {
   public:
    std::vector<Output>
    operator()(const std::vector<Output>& available_outputs, int64_t amount) const;
  };
}  // namespace wallet
