#pragma once

#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>

namespace IO::WDT
{
  class WDTRoot : public Common::Traits::AutoIOTraitInterface<WDTRoot, Common::Traits::TraitType::File>
  {

  };
}
