#pragma once

#include "ui/cli.h"

#include <map>
#include <string>

/** @brief CLI name → example entry point. Implemented with the example sources. */
std::map<std::string, ExampleFn> harmonyCppExampleRegistry();
