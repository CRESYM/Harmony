# Guides for Developers

Audience: people adding **elements**, **solvers**, **tests**, or **examples** to Harmony.

Start here for the wider docs map: [`docs/README.md`](README.md).  
For class/method detail: [Doxygen API docs](doxygen/README.md).  
For end-user workflows: [User Manual](manual/README.md).

## Repository structure

| Path | Role |
|------|------|
| `src/main.cpp` | CLI entry point (`Harmony`) |
| `src/core/` | Shared headers (`Constants.h`, umbrella includes) |
| `src/network/` | `Bus`, `Network`, `SubNetwork` |
| `src/ui/` | CLI, banner, plots, HarmonyUI launcher |
| `src/Elements/` | Device models (`Element` hierarchy) |
| `src/Solver/` | OPF, DQsym, state-space, stability, certificates |
| `src/Control/` | Controllers / filters used by converters |
| `src/json/` | JSON builders, validation, computation runner |
| `cmake/sources.cmake` | Lists of sources / headers for CMake and VS filters |
| `cmake/dependencies.cmake` | Third-party link helpers (Eigen, SymEngine, …) |
| `tests/` | GoogleTest suite (built from the root `CMakeLists.txt`) |

The compiler include root is `src/` only. Quoted includes are paths under that root, for example `"core/Constants.h"`, `"network/network.h"`, `"Elements/Impedance/Resistor.h"`. Same-directory includes (`"Element.h"` next to `Element.cpp`) are still valid.
| `docs/` | User + developer documentation |

## Adding new elements and solvers

1. Implement under `src/Elements/` or `src/Solver/` following nearby types.
2. Register headers/sources in [`cmake/sources.cmake`](../cmake/sources.cmake).
3. If the type should be loadable from JSON, extend `src/json/` builders + [`input-file-format.md`](input-file-format.md).
4. Prefer a small example under `src/examples/` and a unit test under `tests/`.
5. Document the workflow in the [User Manual](manual/README.md) when it is user-facing.

API comments: use Doxygen `/** … */` so they appear in the generated reference ([doxygen/README.md](doxygen/README.md)).

## Adding tests

1. Add `tests/test*.cpp` and list the file in `tests/CMakeLists.txt`.
2. Build `testharmony` from the repository `build/` tree as in [installation.md — Testing](installation.md#testing) / [Manual Ch. 2](manual/02-getting-started.md#27-running-the-test-suite).
3. Keep examples in the `TestExamples` GoogleTest suite so CI can report unit and example results as separate steps.

## Adding examples

1. Add `src/examples/example_*.cpp` and declare it in [`cmake/sources.cmake`](../cmake/sources.cmake) + `Examples.h`.
2. Register the CLI name in [`src/examples/example_registry.cpp`](../src/examples/example_registry.cpp) (see existing `--cpp` entries).
3. Optionally add a mirror JSON under `src/examples/json/`.
4. List it in [Manual Ch. 9](manual/09-examples-catalog.md).

## Statis Analysis with Clang-Tidy

Harmony uses [Clang-Tidy](https://clang.llvm.org/extra/clang-tidy/index.html) to check the code for potential problems and deviations from recommended C++ coding practices. This process is called _static analysis_: the code is inspected without actually running the application.

Clang-Tidy supports many different checks. The checks used in Harmony are specified in the repository's [.clang-tidy](../.clang-tidy) configuration file. The current enabled checks and the number of warnings they produce are:
* `portability`: **7 warnings**. Detects code that may behave differently across platforms, compilers, or architectures.
* `bugprone`: **198 warnings**. Identifies suspicious code constructs and common programming mistakes that could lead to bugs or unexpected behavior. Excludes `bugprone-easily-swappable-parameters`.
* `clang-analyzer`: **26 warnings**. Uses deeper static analysis to detect potential bugs, such as null pointer dereferences, uninitialized values, and incorrect memory access.
* `cppcoreguidelines`: **4178 warnings**. Checks compliance with the C++ Core Guidelines, promoting safer, more maintainable, and more idiomatic C++ code.

**Note**: The warning counts below reflect the results of a static analysis run on the current version of the Harmony codebase (commit `f8a9300d59d`). They are provided as a snapshot and may change as the codebase evolves or the Clang-Tidy configuration is updated.

Clang-Tidy is automatically run on pull requests targeting the `develop` and `main` branches. The check focuses on warnings in code introduced or modified by the pull request, rather than reporting all existing warnings in the codebase. You can also run Clang-Tidy locally to check your changes and identify potential issues before opening a pull request.


### Installing Clang-Tidy

Clang-Tidy can be installed on both Linux and macOS.

<details>
<summary>Install on macOS</summary>

```bash
#  Clang-tidy is bundled inside llvm
brew install llvm

# Add llvm to your path 
echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.bash_profile && source ~/.bash_profile

# Verify the installation
clang-tidy --version        # e.g. Homebrew LLVM version 23.1.3
```
</details>

<details>
<summary>Install on Linux (Ubuntu)</summary>

```bash
sudo apt update
sudo apt install clang-tidy

# Verify the installation
clang-tidy --version
```
</details>

### Running Clang-Tidy

There are two ways to run Clang-Tidy locally:
* **Standalone execution (recommended)**: Run Clang-Tidy separately from the normal build process. This allows you to check the code whenever you want, without having to recompile the entire project.

* **As part of the CMake build**: Enable Clang-Tidy using the `ENABLE_CLANG_TIDY` CMake option. In this mode, Clang-Tidy runs automatically whenever the relevant source files are compiled. This is convenient if you want to check your code regularly, but it increases build times significantly.

### Standalone execution (recommended)

Clang-Tidy needs to know how Harmony is compiled, including which compiler options and external libraries are used. This information is provided through a file called `compile_commands.json`. CMake can generate this file automatically when the `CMAKE_EXPORT_COMPILE_COMMANDS` option is enabled. 

**1. Configure the project**. Run the following commands from the root of the repository, replacing `<path_to_gurobi_installation>` with the path to your local Gurobi installation. For example, on Linux: `/opt/gurobi1301/linux64` and on macOS: `/Library/gurobi1301/macos_universal2`

```bash
# Create a separate build directory for static analysis
mkdir static_analysis

# Configure the project and generate compile_commands.json
cmake -B static_analysis -S . -DGUROBI_PATH=<path_to_gurobi_installation> -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_PRECOMPILED_HEADERS=OFF
```

The options have the following purposes:
* `-B static_analysis`: Stores the generated build files in the `qstatic_analysis` directory, keeping them separate from your normal build.
* `-S .`: Specifies the path to the top-level `CMakeLists.txt` file.
* `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`: Generates `compile_commands.json`, which contains the compiler commands for the project's source files.
* `-DUSE_PRECOMPILED_HEADERS=OFF`: Disables precompiled headers, which can interfere with Clang-Tidy analysis.

**2. Run Clang-Tidy**. Once CMake has finished configuring the project, you can run Clang-Tidy using:

```bash
run-clang-tidy -p buildclangtidy -quiet -header-filter='(^|/)Harmony/src/.*\.h$' -exclude-header-filter='(^|/)stb_image_write\.h$'  '(^|/)Harmony/src/.*\.cpp$' > output.txt
```

This command runs Clang-Tidy on the source files listed in `compile_commands.json`. The options control which files are analysed and where the results are saved:
* `-p static_analysis`: Specifies the directory containing `compile_commands.json`.
* `-quiet`: Suppresses per-file statistics. It does not suppress warnings or errors from Clang-Tidy.
* `-header-filter`: Specifies which header files should be included when reporting warnings. Here, only headers located in `Harmony/src/` are included.
* `-exclude-header-filter`: Excludes `stb_image_write.h`, which is a third-party header and should not be checked.
* `('(^|/)Harmony/src/.*\.cpp$')` selects the `.cpp` source files to analyse. This deliberately excludes the project tests located in `Harmony/tests/`.

Note that header files are checked when they are included by the selected `.cpp` files. They are not independently analysed by this command.

The results are saved in `static_analysis/output.txt`, which you can open with any text editor.

**3. Review the results**. Clang-Tidy reports potential problems in the code, including the file name, line number and a description of each finding. For example, a warning might look like this:

```bash
/path/to/Harmony/src/example.cpp:42:10: warning: ...
```

A warning does not necessarily mean that your code is incorrect or that the application will fail. It indicates something that may deserve attention. Review each finding and determine whether a change is necessary.

The output file can become quite long, especially when analysing the entire project. The following optional command gives you an overview of which source and header files produce the most warnings:

```bash 
grep -E '^[^ ]+\.cpp:[0-9]+:[0-9]+: warning: ' output.txt | sed -E 's/:([0-9]+):[0-9]+: warning:.*//' | sort | uniq -c | sort -nr | awk '{print; total += $1} END {print "TOTAL " total}'
```

This prints the number of warnings associated with each file, sorted from highest to lowest, followed by the total number of warnings.

<details>
<summary>Example output</summary>

```bash
36 /Users/yq/tmp/Harmony/src/ui/Visualization.cpp
23 /Users/yq/tmp/Harmony/src/Solver/OPF/Powerflow_network_params.cpp
20 /Users/yq/tmp/Harmony/src/Elements/Converter/MMC.cpp
18 /Users/yq/tmp/Harmony/src/Solver/Helper_Functions/Symbolic_functions.cpp
15 /Users/yq/tmp/Harmony/src/Solver/Stability_Estimate/Stability_estimate.cpp
13 /Users/yq/tmp/Harmony/src/Elements/Transmission_Line/Cable.cpp
10 /Users/yq/tmp/Harmony/src/Solver/Helper_Functions/Standard_functions.cpp
10 /Users/yq/tmp/Harmony/src/Solver/Certificate/Stability_certificate.cpp
8 /Users/yq/tmp/Harmony/src/Solver/Helper_Functions/Differential_equations.cpp
8 /Users/yq/tmp/Harmony/src/Solver/DQsym/DQsym.cpp
7 /Users/yq/tmp/Harmony/src/Solver/OPF/Powerflow_solver.cpp
6 /Users/yq/tmp/Harmony/src/Solver/State_Space_Model/State_Space_Model.cpp
6 /Users/yq/tmp/Harmony/src/Solver/Helper_Functions/DQsym_Conversion_Functions.cpp
5 /Users/yq/tmp/Harmony/src/ui/harmony_launcher.cpp
4 /Users/yq/tmp/Harmony/src/Elements/Source/Source_base.cpp
3 /Users/yq/tmp/Harmony/src/json/computation_runner.cpp
3 /Users/yq/tmp/Harmony/src/Elements/Transmission_Line/overhead_line.cpp
3 /Users/yq/tmp/Harmony/src/Elements/Impedance/Impedance_base.cpp
2 /Users/yq/tmp/Harmony/src/Solver/OPF/Powerflow_data.cpp
2 /Users/yq/tmp/Harmony/src/Solver/Certificate/Operating_region.cpp
2 /Users/yq/tmp/Harmony/src/examples/example_DQsym_math_operations.cpp
2 /Users/yq/tmp/Harmony/src/Elements/Transmission_Line/Transmissionline.cpp
2 /Users/yq/tmp/Harmony/src/Elements/Load/Load.cpp
2 /Users/yq/tmp/Harmony/src/Elements/Element.cpp
1 /Users/yq/tmp/Harmony/src/examples/example_WT_type_4.cpp
1 /Users/yq/tmp/Harmony/src/examples/example_PV_plant.cpp
1 /Users/yq/tmp/Harmony/src/examples/example_MMC_gfm.cpp
1 /Users/yq/tmp/Harmony/src/examples/example_generator.cpp
1 /Users/yq/tmp/Harmony/src/Elements/RES/WT_type_3.cpp
1 /Users/yq/tmp/Harmony/src/Elements/RES/RES_base.cpp
1 /Users/yq/tmp/Harmony/src/Elements/RES/PV_plant.cpp
TOTAL 217
```
</details>

### Running Clang-Tidy as part of the CMake build

Alternatively, you can enable Clang-Tidy directly in the CMake configuration. In this mode, Clang-Tidy runs automatically whenever a source file is compiled. This is useful if you want to check your code regularly during development.

Start with a clean build directory. If you have already used the `build` directory for a previous compilation, delete its contents before proceeding. CMake stores configuration settings in the build directory, and previously compiled files may not be checked again if they do not need to be recompiled.

To enable Clang-Tidy, configure Harmony with the `ENABLE_CLANG_TIDY` option:

```bash
cd build
cmake .. -DGUROBI_PATH=<path_to_gurobi_installation> -DENABLE_CLANG_TIDY=ON
```

Then build the project as usual:

```bash
cmake --build . --config Release -j
```

Clang-Tidy will now run automatically as part of the compilation process. Its findings will appear in the terminal output alongside the normal compiler messages.

**Note**: Running Clang-Tidy as part of the build increases compilation time. It is therefore generally recommended to use standalone execution when you only want to check your changes occasionally, for example before opening a pull request.

## Related documentation

- [Docs index](README.md)
- [Installation](installation.md) · [Running](running-harmony.md)
- [Maintainer guide](maintainer-guide.md) — CMake, CI, dependencies
