# Harmony $~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~$    ![alt text](cresym.png?raw=true)  

HARMONic stabilitY assessment of PE-penetrated power systems

As other power systems, the European grid experiences a massive RES development, bringing up many new challenges for system operation. One of such challenges is the massive penetration of Power Electronic (PE) converters (e.g. PV & wind generation units, batteries & chargers, HVDC connections, statcoms, etc.) on a large scale as they may cause instable system operation (resonance) or harmonic waves that can interfere and/or damage the neighbouring equipment. 
Where a few, local (a plant) and rather simple situations had to be analysed in the past, the risk is growing exponentially, as more devices connect, as they are more diverse, as they are closer one to another. Grid regulations are all the more demanding that complex situations can actually not be addressed. And, as a consequence, strong mitigation measures are integrated in every piece of equipment, at a significant additional cost… without preventing all undesired interactions.    
There is a small number of publicly available, but limited, tools for such analyses, often requiring time-consuming modeling of the power system and the detailed control algorithms, possibly limited in terms of phasor-based simulation or unable to model all power converters, or unable to satisfactorily model an AC-DC hybrid system. 
Harmony (“HARMONic stabilitY assessment of PE-penetrated power systems”) project deals with the development of a mathematical framework capable of simulating all components in AC-DC system for converter driven stability assessment studies becomes crucial, in order to complement present tools, and eventually:  
(i) ease the performance of stability assessment studies; 
(ii) analyse complex situations; 
(iii) refine mitigation measures; and  
(iv) lower overall equipment costs. 

## Installation and Usage

See the [detailed installation instructions](docs/installation.md).

### Prerequisites 
Harmony can be compiled on Windows, Linux and MacOS. The requirements for each operating system are:
- Compilers for C and C++
    - Linux: GNU C and C++ compilers
    - MacOS: Apple Clang C and C++ compilers
    - Windows: [Visual Studio](https://visualstudio.microsoft.com) - version 2022 or newer, with a "Desktop Development for C++" workload installed (See [installation docs.](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022))
- [miniconda](https://www.anaconda.com/docs/getting-started/miniconda/main) - a miniature version of Anaconda that includes only conda, Python and other few packages. (You can also use Anaconda if it is already installed on your machine.)
> [!WARNING]
> When installing miniconda, make sure you select the option **Add to path**. This will allow you to use miniconda from Visual Studio, Git Bash, etc. If you forgot this, have a look at Step 3 in [this blog post](https://eduand-alvarez.medium.com/setting-up-anaconda-on-your-windows-pc-6e39800c1afb), which shows how to add miniconda to your path. Alternatively, you may reinstall miniconda.

### Build and Run
To build and run Harmony, open Visual Studio and select "Continue without code" on the welcome dialogue. From the top menu, select View->Terminal. This will open the Visual Studio developer prompt. From there, type the following commands:

```bash
# Donwload the repository
git clone https://github.com/CRESYM/Harmony.git
cd Harmony

# Create conda environment with dependencies
conda env create -f environment.yml
conda activate harmony

# Configure the project
mkdir build
cd build
cmake ..

# Compile Harmony
cmake --build . --config Release

# Run Harmony
cd Release  # Windows only
./Harmony
```



## Contributors
- **Aleksandra Lekić**, development of the mathematical framework, development of individual components, and their spectral representation and formulation, and design of the toolbox, different functionality interconnections, harmonic stability solver, supervision, and acquisition of the funding
- **Haixiao Li**, design of the power flow strategy, and formulation of the power flow solution
- **Saif Alsarayreh**, dynamic phasor formulation, model design, and implementation
- **Yasel Quintero Lares**, documentation, installation setup, cross-platform compatibility, issuing, release
- **Azadeh Kermansaravi**, development of the core of the toolbox
- **Robert Dimitrovski**, dynamic phasor formulation, and supervision


 The development of the Harmony was supported by the [Digital Competence Centre](https://www.tudelft.nl/index.php?id=67120&L=1/), Delft University of Technology.
