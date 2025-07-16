#include "Filter.h"

void Filter::updateParameters(double newTimeConstant, double newZeta, double newBandwidth) {
    timeConstant = newTimeConstant;
    zeta = newZeta;
    bandwidth = newBandwidth;
}