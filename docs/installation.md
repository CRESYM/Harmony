# Installation Instructions

This document describes how to set up a development environment that allows you to compile and run Harmony.

## Prerequisites 
Harmony can be compiled on Windows, Linux (tested in Ubuntu 22.04.5), and macOS (tested in Sequoia 15.5). The requirements for each operating system are:
- Compilers for C and C++
    - Linux: GNU C and C++ compilers (tested version with 11.4.0)
    - MacOS: Apple Clang C and C++ compilers (tested with version 17.0.0)
    - Windows: [Visual Studio](https://visualstudio.microsoft.com) - version 2022 or newer, with a "Desktop Development for C++" workload installed (See [installation docs.](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022))
- [Gurobi C++ API](https://docs.gurobi.com/projects/optimizer/en/current/reference/cpp.html): C++ API for optimization (license required, free for academics).
- [miniconda](https://www.anaconda.com/docs/getting-started/miniconda/main) - a miniature version of Anaconda that includes only conda, Python, and a few other packages. (You can also use Anaconda if it's already installed on your machine.)


> [!WARNING]
> When installing miniconda, make sure you select the option **Add to path**. This will allow you to use miniconda from Visual Studio, Git Bash, etc. If you forgot this, have a look at Step 3 in [this blog post](https://eduand-alvarez.medium.com/setting-up-anaconda-on-your-windows-pc-6e39800c1afb), which shows how to add miniconda to your path. Alternatively, you may reinstall miniconda.

<details>

<summary>Verify miniconda installation on Windows</summary>

Open Visual Studio. Close the Welcome dialogue or select the option *Continue without code*. Open the VS built-in terminal by selecting View->Terminal on the top menu. 

The terminal should display `(base)` to the left of the prompt symbol as shown in the picture below. This symbol indicates that VS can find and use miniconda. The word "base" means that no conda environments are activated; if an environment were activated, its name would appear instead of "base".

![Screenshot 2025-05-05 at 21 03 07](https://github.com/user-attachments/assets/f519969a-9976-4255-b226-6aa5a976f190)

**Troubleshooting**

If `(base)` is not displayed, attempt any of the following:
- Close and reopen Visual Studio. 
- With Visual Studio closed, open Windows PowerShell in admin mode. To do so: click on the Windows button in the toolbar, look for Windows PowerShell in the list of installed apps, right-click on it, and select *More -> Run as Administrator*. Enter your username and password to validate your admin privileges. Run the following command, close the PowerShell, and re-open Visual Studio:
```
Set-ExecutionPolicy RemoteSigned
```
</details>


## Installation

**Linux and MacOS:** open the terminal.

**Windows:** Open Visual Studio. Close the Welcome dialogue or select the option *Continue without code*. Open the Visual Studio built-in terminal by selecting View->Terminal on the top menu.

**1. Clone the Harmony repository**, if you haven't yet done so. You can clone the repository by executing the commands: 
```bash
# Download repository
git clone https://github.com/CRESYM/Harmony.git
# Open the folder
cd Harmony
``` 
If you have already cloned the repo, open it using the `cd <path-to-harmony>` command, for example: `cd C:\Users\<netid>\Desktop\Harmony`

After opening the Harmony repository, use the `ls` command to verify you are in the correct directory. All of Harmony's files should be printed in the terminal when executing the command.

**2. Create the conda environment** with all of Harmony's dependencies. Creating the environment <ins>only needs to be done once</ins>. The file [environment.yml](../environment.yml) specifies the name of the environment and the libraries that will be installed within it. To create the environment, run the following command from the terminal: 
``` bash
# Create the harmony conda environment (done once)
conda env create -f environment.yml
```

The `conda env list` command prints the names of the conda environments installed on your machine. You can use it to verify that the harmony environment was created successfully. See [Managing Conda Environments](https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html) for guidance on how to work with conda environments.


**3. Activate the conda environment** created in the previous step. You will need to activate the environment every time you want to compile or execute Harmony. Activating the environment allows the compiler to find Harmony's dependencies. To activate it, run: 
``` bash
# Activate the harmony conda environment (done every time you want to compile or run harmony)
conda activate harmony
```

If the environment was correctly activated, `(harmony)` should be displayed to the left of the terminal's prompt. Use `conda deactivate` to deactivate the harmony environment - this will replace `(harmony)` with `(base)` on the terminal's prompt. 

![Screenshot 2025-05-05 at 21 14 39](https://github.com/user-attachments/assets/2c430c03-0438-4afa-95f9-75d697d7642a)

> [!TIP]
> Always make sure `(harmony)` is displayed on the terminal prompt before attempting to compile or run harmony. This symbol indicates the environment is activated.

**4. Use CMake** to create the Harmony Visual Studio Solution on Windows or the Harmony makefile on Linux and MacOS. The file [CMakeLists.txt](../CMakeLists.txt) contains the instructions to create the VS Solution and makefile.
```bash
# Create a build directory where the VS Solution or makefile will be generated
mkdir build

# Open the build directory
cd build

# Generate the VS Solution or makefile. CMake will look for CMakeLists.txt in the parent directory of the build folder (indicated by ..)
# Specify the path to your gurobi installation, for example -DGUROBI_PATH="C:\gurobi1202\win64"
cmake .. -DGUROBI_PATH="gurobi_installation_dir_config" 
```

The VS Solution or makefile will be inside the `build` directory. The VS Solution or makefile needs to be regenerated every time you make changes to *CMakeLists.txt*. These changes can include, for example: adding a new *.h* or *.cpp* file to the project, adding a compiler flag or a dependency on a third-party library, etc. To regenerate the solution or makefile, delete the `build` folder and re-run the commands listed in this step.

**5. Make changes to the code (optional)**.
- For Windows users, open the Visual Studio solution generated in the previous step. Do so by selecting File->Open->Project/Solution on the top menu. Choose the file `<path-to-harmony/build/Harmony.sln>`. Click View->"Solution Explorer" on the top menu to display the project's source files. Note: Harmony will add the ALL_BUILD and ZERO_CHECK projects to the VS solution. You can ignore them.
- For macOS and Linux users, open the project's source files in the IDE of your choice.

Carry out your development as usual, making changes to the project's source files and saving them.

**6. Compile Harmony.** The following command (which needs to be run from the `build` folder containing the VS Solution or makefile) will build Harmony in Release mode. Use `--config Debug` to build in Debug mode. The `-j 4` flag enables parallel compilation on Linux and macOS using the specified number of cores, 4 in this case. For Windows, Harmony is already configured (via CMake) to automatically detect and use the maximum number of available cores for parallel compilation.

```bash
# Replace 4 with the number of CPU cores you wish to use for parallel compilation
cmake --build . --config Release -j 4
```

Windows users: You may also use the VS Build button or select Build->Solution from the menu at the top. The output file *Harmony.exe* will be created in the `build/Release` directory or `build/Debug` if the app was built in Debug mode.

**11. Run Harmony.**
```bash
# Windows only: open the folder containing Harmony.exe
cd Release

# Run Harmony
./Harmony
```

Windows users: If you want to use the Visual Studio "Run" button or menu options, you need to set the Harmony project as the startup project in the Solution (CMake adds ALL_BUILD and ZERO_CHECK projects). To do so: right-click on the Harmony project in the *Solution Explorer* and select the option *Set as Startup Project*.
