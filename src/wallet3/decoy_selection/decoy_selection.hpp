#pragma once

#include <vector>
#include "../daemon_comms.hpp"
#include "../output.hpp"
#include "../decoy.hpp"

namespace wallet
{
  // DecoySelector will choose some a subset of outputs from the provided list of outputs according
  // to the decoy selection algorithm. The decoys selected should hide the selected output within a 
  // ring signature and requires careful selection to avoid privacy decreasing analysis

  class DecoySelector
  {
   public:
    std::vector<Decoy>
    operator()(const Output& selected_output);

    DecoySelector(std::shared_ptr<DaemonComms> dmn) : daemon(std::move(dmn)) {};

   private:
    std::shared_ptr<DaemonComms> daemon;

  };
}  // namespace wallet
