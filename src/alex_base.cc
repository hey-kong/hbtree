#include "alex_base.h"

namespace alex {

/*** Cost model weights ***/

// Intra-node cost weights
double kExpSearchIterationsWeight = 20;
double kShiftsWeight = 0.5;

// TraverseToLeaf cost weights
double kNodeLookupsWeight = 20;
double kModelSizeWeight = 5e-7;

// https://en.wikipedia.org/wiki/CPUID#EAX=7,_ECX=0:_Extended_Features
bool cpu_supports_bmi() {
  return static_cast<bool>(CPUID(7, 0).EBX() & (1 << 3));
}

}  // namespace alex
