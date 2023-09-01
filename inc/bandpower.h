#include "hammingWindow.h"
#include "sigpack.h"
using namespace arma;
using namespace sp;

void bandpower(std::vector<std::vector<float>>& input, int rangeLow, int rangeHigh, std::vector<float>& output);