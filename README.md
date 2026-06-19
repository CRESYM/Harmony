![Windows build & test](https://github.com/CRESYM/Harmony/actions/workflows/windows_build_and_test.yml/badge.svg?branch=main) ![Linux build & test](https://github.com/CRESYM/Harmony/actions/workflows/linux_build_and_test.yml/badge.svg?branch=main) ![macOS build & test](https://github.com/CRESYM/Harmony/actions/workflows/macos_build_and_test.yml/badge.svg?branch=main)   
![Windows run examples](https://github.com/CRESYM/Harmony/actions/workflows/windows_run_examples.yml/badge.svg?branch=main) ![Linux run examples](https://github.com/CRESYM/Harmony/actions/workflows/linux_run_examples.yml/badge.svg?branch=main) ![macOS run examples](https://github.com/CRESYM/Harmony/actions/workflows/macos_run_examples.yml/badge.svg?branch=main)

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

## Installation and Usage

See the [detailed installation instructions](docs/installation.md), the [User Manual](docs/manual/README.md), and the [API documentation (Doxygen)](docs/doxygen/README.md).

### Prerequisites 
Harmony can be compiled on Windows. The requirements are:
- [Gurobi Optimizer](https://www.gurobi.com/downloads/gurobi-software/?_gl=1*nfc3bz*_up*MQ..*_ga*Mzk5NjUzMDE0LjE3NDk3NDM5OTU.*_ga_RTTPP25C8N*czE3NDk3NDM5OTQkbzEkZzEkdDE3NDk3NDQxMTIkajYwJGwwJGgxNzI0MDAwOTc3) - requires a license (free for academics).
- [Visual Studio](https://visualstudio.microsoft.com) - version 2022 or newer, with a "Desktop Development for C++" workload installed (See [installation docs.](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022))
- [miniconda](https://www.anaconda.com/docs/getting-started/miniconda/main) - a miniature version of Anaconda that includes only conda, Python, and a few other packages. (You can also use Anaconda if it is already installed on your machine.)
  
> [!WARNING]
> When installing miniconda, make sure you select the option **'Add to path'**. This will allow you to use miniconda from Visual Studio, Git Bash, etc. If you forgot this, have a look at Step 3 in [this blog post](https://eduand-alvarez.medium.com/setting-up-anaconda-on-your-windows-pc-6e39800c1afb), which shows how to add miniconda to your path. Alternatively, you may reinstall miniconda.


### Build and Run
To build and run Harmony, open Visual Studio and select "Continue without code" on the welcome dialogue. From the top menu, select View->Terminal. This will open the Visual Studio Developer Command Prompt. From there, type the following commands:

```bash
# Download the repository
git clone https://github.com/CRESYM/Harmony.git
cd Harmony
# If you want to run the code with an input file, go to the folder input_file
# (cd Harmony/src/input_file) and follow the rest of the instructions

# Create conda environment with dependencies
conda env create -f environment.yml
conda activate harmony

# Configure the project
mkdir build
cd build
cmake .. -DGUROBI_PATH="gurobi_installation_dir_config" 
# e.g. cmake .. -DGUROBI_PATH="C:\gurobi1202\win64" 

# Compile Harmony
# Replace 4 with the number of CPU cores you wish to use for parallel compilation
cmake --build . --config Release -j 4

# Run Harmony
cd Release
./Harmony
```

> [!TIP]
> Adding the `-j` flag enables parallel compilation on Linux and macOS using the specified number of cores. For Windows, Harmony is already configured to automatically detect and use the maximum number of available cores for parallel compilation.

### Run the tests
To run the tests, you should follow a similar procedure to building and running the code.
```bash
# Open the tests directory
cd tests

# Activate the harmony conda environment
conda activate harmony

# Configure the project
mkdir build
cd build
cmake .. -DGUROBI_PATH="gurobi_installation_dir_config" 
# e.g. cmake .. -DGUROBI_PATH="C:\gurobi1202\win64"

# Compile the tests
cmake --build . --config Release

# Run the tests
ctest
```


## Contributors
- **Aleksandra Lekić**, development of the mathematical framework, development of individual components, and their spectral representation and formulation, design of the toolbox, different functionality interconnections, harmonic stability solver, supervision, and acquisition of the funding
- **Robert Dimitrovski**, dynamic phasor formulation, development of a core of DQsym, funding acquisition and supervision
- **Haixiao Li**, design of the power flow strategy, and formulation of the power flow solution
- **Saif Alsarayreh**, dynamic phasor formulation, model design, and implementation
- **Azadeh Kermansaravi**, programming of the part of the toolbox
- **Yasel Quintero**, documentation, installation setup, cross-platform compatibility, testing, issuing, release




 The development of the Harmony was supported by the [Digital Competence Centre](https://www.tudelft.nl/index.php?id=67120&L=1/), Delft University of Technology.

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
