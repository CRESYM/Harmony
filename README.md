![Windows build & test](https://github.com/CRESYM/Harmony/actions/workflows/windows_build_and_test.yml/badge.svg?branch=main) ![Linux build & test](https://github.com/CRESYM/Harmony/actions/workflows/linux_build_and_test.yml/badge.svg?branch=main) ![macOS build & test](https://github.com/CRESYM/Harmony/actions/workflows/macos_build_and_test.yml/badge.svg?branch=main)   
![Windows run examples](https://github.com/CRESYM/Harmony/actions/workflows/windows_run_examples.yml/badge.svg?branch=main) ![Linux run examples](https://github.com/CRESYM/Harmony/actions/workflows/linux_run_examples.yml/badge.svg?branch=main) ![macOS run examples](https://github.com/CRESYM/Harmony/actions/workflows/macos_run_examples.yml/badge.svg?branch=main)  
![Build API docs](https://github.com/CRESYM/Harmony/actions/workflows/docs.yml/badge.svg?branch=main)

# Harmony $~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~$    ![alt text](cresym.png?raw=true)  

HARMONic stabilitY assessment of PE-penetrated power systems

Like other power systems, the European grid experiences a massive RES development, bringing up many new challenges for system operation. One of such challenges is the massive penetration of Power Electronic (PE) converters (e.g., PV & wind generation units, batteries & chargers, HVDC connections, statcoms, etc.) on a large scale, as they may cause unstable system operation (resonance) or harmonic waves that can interfere and/or damage the neighbouring equipment. 

Where a few, local (a plant) and rather simple situations had to be analyzed in the past, the risk is growing exponentially as more devices connect, become more diverse, and are closer to one another. Grid regulations are all the more demanding because complex situations cannot be addressed directly. And, as a consequence, strong mitigation measures are integrated in every piece of equipment, at a significant additional cost… without preventing all undesired interactions.    

There is a limited number of publicly available tools for such analyses, often requiring time-consuming modeling of the power system and detailed control algorithms, which may be limited in terms of phasor-based simulation, unable to model all power converters, or unable to satisfactorily model an AC-DC hybrid system. 
Harmony (“HARMONic stabilitY assessment of PE-penetrated power systems”) project deals with the development of a mathematical framework capable of simulating all components in AC-DC systems for converter-driven stability assessment studies becomes crucial, in order to complement present tools, and eventually:  

(i) ease the performance of stability assessment studies; 

(ii) analyse complex situations; 

(iii) refine mitigation measures; and 

(iv) lower overall equipment costs. 

![alt text](/install/res/HARMONY_figure.png?raw=true) 

If you use this repository, please cite the following publication.
```
@misc{lekic2026harmony,
  title         = {Advanced Simulation Framework for AC/MTDC Power Systems},
  author        = {Aleksandra Leki{\'c} and Azadeh Kermansaravi and Haixiao Li and Yasel Quintero Lares and Saif Alsarayreh and Robert Dimitrovski},
  year          = {2026},
  eprint        = {2606.09406},
  archivePrefix = {arXiv},
  primaryClass  = {eess.SY},
  url           = {https://arxiv.org/abs/2606.09406}
}
```

## Installation and Usage

### Prerequisites 

Harmony can be compiled on Windows, Linux and macOS. The requirements for each operating system are:
- Compilers and build tools for C and C++
    - Linux: GNU C and C++ compilers (tested version with 11.4.0) and GNU make
    - MacOS: Apple Clang C and C++ compilers (tested with version 21.0.0) and GNU make
    - Windows: [Visual Studio](https://visualstudio.microsoft.com) - version 2022 or newer, with a "Desktop Development for C++" workload installed (See [installation docs.](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022))
- [miniconda](https://www.anaconda.com/docs/getting-started/miniconda/main) - a miniature version of Anaconda that includes only conda, Python, and a few other packages. (You can also use Anaconda if it's already installed on your machine.)
- [Gurobi](https://www.gurobi.com/product/download-center/optimizer-software) - optimization software (license required, free for academics).
- Additional system packages (Linux only):
    - ```bash
      # Install the OpenGL, X11, and GLFW development libraries
      sudo apt update
      sudo apt install libgl1-mesa-dev libx11-dev libglfw3-dev
      ```

> [!WARNING]
> When installing miniconda, make sure you select the option **Add to path**. This will allow you to use miniconda from Visual Studio, Git Bash, etc. If you forgot this, have a look at Step 3 in [this blog post](https://eduand-alvarez.medium.com/setting-up-anaconda-on-your-windows-pc-6e39800c1afb), which shows how to add miniconda to your path. Alternatively, you may reinstall miniconda.


### Building

See the [detailed installation instructions](docs/installation.md) for a comprehensive, step-by-step guide to building Harmomy.   

To build and run Harmony:
* Linux and MacOS: open the terminal and type the commands below.
* Windows: open Visual Studio and select "Continue without code" on the welcome dialogue. From the top menu, select View->Terminal. This will open the Visual Studio Developer Command Prompt. From there, type the following commands.

```bash
# Download the repository
git clone https://github.com/CRESYM/Harmony.git
cd Harmony

# Create conda environment with dependencies
conda env create -f environment.yml
conda activate harmony

# Create a build directory for compilation
mkdir build
cd build

# Configure the CMake project specifying the path to your Gurobi installation, for example:
#   Windows:  cmake .. -DGUROBI_PATH="C:/gurobi1202/win64"
#   Linux:    cmake .. -DGUROBI_PATH="/opt/gurobi1301/linux64"
#   MacOS:    cmake .. -DGUROBI_PATH="/Library/gurobi1200/macos_universal2"
cmake .. -DGUROBI_PATH="gurobi_installation_dir_config"

# Compile Harmony
# Replace 4 with the number of CPU cores you wish to use for parallel compilation
cmake --build . --config Release -j 4
```

> [!TIP]
> Adding the `-j` flag enables parallel compilation on Linux and macOS using the specified number of cores. For Windows, Harmony is already configured to automatically detect and use the maximum number of available cores for parallel compilation.

### Running

See the [detailed running instructions](docs/running-harmony.md) for a for a comprehensive guide to running Harmony.   

Compiling Harmony produces two executables:
* **HarmonyUI**: Run simulations via a graphical user interface (GUI). Simulations are configured via JSON input files.
* **Harmony**: Run simulations from the command-line interface (CLI). Simulations are configured via JSON input files or C++ scripts.

**Using the graphical user interface**   

To use Harmony's graphical user interface, execute the following command from the `build` directory:
```bash
# Run HarmonyUI (Windows)
./Release/HarmonyUI.exe

# Run HarmonyUI (Linux and MacOS)
./HarmonyUI
```

**Using the command-line interface**

To use Harmony's command-line interface, run this command from the `build` directory:"
```bash
# Run HarmonyUI (Windows)
./Release/Harmony.exe <option>

# Run HarmonyUI (Linux and MacOS)
./Harmony <option>
```

Replace `<option>` with the actual option you would like to use. The available options are described in the [detailed running instructions](docs/running-harmony.md).



<!-- This part is commented-off

```bash
conda activate harmony
cd ..                              # repository root (from build/)

# Windows
build\Release\Harmony.exe --help
build\Release\Harmony.exe --list-cpp
build\Release\Harmony.exe --cpp stability_check
build\Release\Harmony.exe --json src/examples/json/stability_check.json

# Linux / macOS
./build/Harmony --help
./build/Harmony --cpp stability_check
./build/Harmony --json src/examples/json/stability_check.json
```

| Goal | HarmonyUI | Harmony (CLI) |
|------|-----------|---------------|
| Interactive runs | Launcher → **Run** | `Harmony --cpp <name>` or `--json <file>` |
| List examples | Dropdown menus | `--list-cpp`, `--list-json` |
| No plots | Leave **Plot** unchecked | add `--no-plot` |
| More output | **Verbose log** checkbox | add `--verbose` |

See [User Manual — HarmonyUI](docs/manual/11-harmony-ui.md) for the full GUI guide.

-->

### Building and running the tests

The procedure to compile the tests is very similar to the one used to compile Harmomy. To run the tests, execute the following commands, starting from the *root level* of the repository, with the harmony conda environment activated:

```bash
# Open the tests directory
cd tests

# Create a build directory
mkdir build
cd build

# Configure the CMake project specifying the path to your Gurobi installation, for example:
#   Windows:  cmake .. -DGUROBI_PATH="C:/gurobi1202/win64"
#   Linux:    cmake .. -DGUROBI_PATH="/opt/gurobi1301/linux64"
#   MacOS:    cmake .. -DGUROBI_PATH="/Library/gurobi1200/macos_universal2"
cmake .. -DGUROBI_PATH="gurobi_installation_dir_config" 

# Compile the tests. 
# Replace 4 with the number of CPU cores you wish to use for parallel compilation
cmake --build . --config Release -j 4

# Run the tests
ctest -j 4
```


## Contributors
- **Aleksandra Lekić**, development of the mathematical framework, development of individual components, and their spectral representation and formulation, design of the toolbox, different functionality interconnections, harmonic stability solver, supervision, and acquisition of the funding
- **Robert Dimitrovski**, dynamic phasor formulation, development of a core of DQsym, funding acquisition and supervision
- **Haixiao Li**, design of the power flow strategy, and formulation of the power flow solution
- **Saif Alsarayreh**, dynamic phasor formulation, model design, and implementation
- **Azadeh Kermansaravi**, programming of the part of the toolbox
- **Yasel Quintero**, documentation, installation setup, cross-platform compatibility, testing, issuing, release




 The development of the Harmony was supported by the [Digital Competence Centre](https://dcc.tudelft.nl/), Delft University of Technology.

 ## Technical Foundations

Built on component models and state-space methods from prior work:
- Transmission line, MMC base models: PowerImpedanceACDC.jl (A. Lekić, 2024)
- State-space solver: dc_dc_simulator (A. Lekić, GitHub)
- AC-DC OPF solver: ACDC-OpFlow (H. Li, GitHub)
- Dynamic phasor solver: DQsym (S. Alsarayreh, R. Dimitrovski, GitHub)

All analysis solvers, optimization routines, and stability assessment methods are original developments for this framework.

*License*: GPL v3

### References

[1] ELECTA, PowerImpedanceACDC-Impedance-based stability analyses, "PowerImpedanceACDC.jl," 2024, https://github.com/Electa-Git/PowerImpedanceACDC.jl

[2] Lekić, A., "DC-DC Simulator," https://github.com/kul-optec/dc_dc_simulator

[3] Li, H., Kermansaravi, A., Dimitrovski, R., & Lekić, A. (2025). ACDC-OpFlow, Unified, Cross-Language Framework for AC/DC Optimal Power Flow Solutions (Version v0.1) [Computer software]. https://doi.org/10.4121/66318317-4d5d-4dc4-ba5a-5fa65c585520

[4] Alsarayreh, S., Dimitrovski, R., & Lekić, A. (2025). DQsym: A Dynamic Phasor-Based library for Analysis of Modern Power Systems This repository presents the Dynamic Phasor library for Analysis of Modern Power Systems (Version 1.0.0) [Computer software]. https://doi.org/https://doi.org/10.5281/zenodo.18544532
