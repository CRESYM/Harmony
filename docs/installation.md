# Installation Instructions

This document describes how to set up a development environment that allows you to compile and run Harmony.

After building, see [**Running Harmony**](running-harmony.md) for command-line usage (`--cpp`, `--json`, flags, and troubleshooting).

## Prerequisites 
Harmony can be compiled on Windows, Linux (tested in Ubuntu 22.04.5), and macOS (tested in Tahoe 26.5). The requirements for each operating system are:
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

<details>

<summary>Verify miniconda installation on Windows</summary>

Open Visual Studio (VS). Close the Welcome dialogue or select the option *Continue without code*. Open the VS built-in terminal by selecting View->Terminal on the top menu. 

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

**Windows:** Open Visual Studio (VS). Close the Welcome dialogue or select the option *Continue without code*. Open the Visual Studio built-in terminal by selecting View->Terminal on the top menu.

**1. Clone the Harmony repository** by running the following commands from the terminal: 
```bash
# Download repository
git clone https://github.com/CRESYM/Harmony.git
# Open the folder
cd Harmony
```
If you had previously cloned the repository you can skip this step and simply navigate to the existing Harmony directory using the `cd <path-to-harmony>` command, for example: `cd C:\Users\<netid>\Desktop\Harmony`

After opening the Harmony repository, use the `ls` command to verify you are in the correct directory. All of Harmony's files should be printed in the terminal when executing the command.

**2. Create the conda environment** with all of Harmony's dependencies. Creating the environment <ins>only needs to be done once</ins>. The file [environment.yml](../environment.yml) specifies the environment name and the libraries that will be installed within it. To create the environment, run the following command from the terminal: 
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

> [!TIP]
> Always make sure `(harmony)` is displayed on the terminal prompt before attempting to compile or run harmony. This symbol indicates the environment is activated.

![Screenshot 2025-05-05 at 21 14 39](https://github.com/user-attachments/assets/2c430c03-0438-4afa-95f9-75d697d7642a)  
*Example: Windows terminal.*

**4. Use CMake** to create the Harmony Visual Studio Solution on Windows or the Harmony makefile on Linux and MacOS. The file [CMakeLists.txt](../CMakeLists.txt) contains the instructions to create the VS Solution and makefile.
```bash
# 4.1 Create a directory called build
mkdir build

# 4.2 Open the build directory
cd build

# 4.3 Generate the VS Solution or makefile
#     CMake will look for CMakeLists.txt in the parent directory of the build folder (indicated by ..)
#     Specify the path to your Gurobi installation, for example:
#     Windows:  cmake .. -DGUROBI_PATH="C:/gurobi1202/win64"
#     Linux:    cmake .. -DGUROBI_PATH="/opt/gurobi1301/linux64"
#     macOS:    cmake .. -DGUROBI_PATH="/Library/gurobi1200/macos_universal2"

cmake .. -DGUROBI_PATH="gurobi_installation_dir_config"
```

The VS Solution or makefile will be inside the `build` directory. The VS Solution or makefile needs to be regenerated every time you make changes to `CMakeLists.txt`. These changes can include, for example: adding a new `.h` or `.cpp` file to the project or adding a dependency on a third-party library. To regenerate the solution or makefile, re-run the `cmake .. -DGUROBI_PATH="gurobi_installation_dir_config"` command from the `build` folder.

**5.** (Optional, Windows) Open the VS Solution created in the previous step. Do so by selecting File->Open->Project/Solution on the top menu. Choose the file `<path-to-harmony/build/Harmony.sln>`. Carry out your development as usual, making changes to the project's source files and saving them.

Note: Harmony will add the ALL_BUILD and ZERO_CHECK projects to the VS solution. You can ignore them.

**6. Compile Harmony.** The following command (which needs to be run from the `build` folder containing the VS Solution or makefile) will build Harmony in Release mode.  

Use `--config Debug` to build in Debug mode (currently not supported). Replace `<num-cores>` with the number of CPU cores you wish to use for parallel compilation, e.g. `-j 4`. For Windows, Harmony is already configured to automatically detect and use the maximum number of available cores for parallel compilation.
```bash
cmake --build . --config Release -j <num-cores>
```

Windows users: You may also use the VS Build button or select Build->Solution from the menu at the top. The output files *Harmony.exe* and *HarmonyUI.exe* are created in the `build/Release` directory (or `build/Debug` in Debug mode).

**7. Execute Harmony.**

Building Harmony produces two executables:

| Executable | Build target | Use |
|------------|--------------|-----|
| **HarmonyUI** | `HarmonyUI` | Graphical launcher (recommended for interactive use) |
| **Harmony** | `Harmony` | Command-line interface for developers and scripts |

```bash
conda activate harmony
cd ..    # repository root (from build/)

# Build HarmonyUI (optional; included in a full build)
cmake --build build --config Release --target HarmonyUI

# Windows (graphical launcher)
build\Release\HarmonyUI.exe

# Windows (CLI)
build\Release\Harmony.exe --help
build\Release\Harmony.exe --cpp stability_check
build\Release\Harmony.exe --json src/examples/json/stability_check.json

# Linux / macOS (graphical launcher)
./build/HarmonyUI

# Linux / macOS (CLI)
./build/Harmony --help
./build/Harmony --cpp stability_check
./build/Harmony --json src/examples/json/stability_check.json
```

| Goal | HarmonyUI | Harmony (CLI) |
|------|-----------|---------------|
| Pick and run examples | Launcher → **Run** | `Harmony --cpp <name>` |
| Run JSON case | Launcher → JSON → **Run** | `Harmony --json <file>` |
| Disable plots | Leave **Plot** unchecked | `--no-plot` |
| List examples | Dropdown menus | `--list-cpp`, `--list-json` |

Full details: [Running Harmony](running-harmony.md), [User Manual — HarmonyUI](manual/11-harmony-ui.md).

**Visual Studio:** Set **HarmonyUI** as the startup project for the GUI, or **Harmony** for CLI debugging. Set **Working Directory** to the repository root. For **Harmony**, add **Command Arguments** such as `--cpp stability_check --no-plot` or `--json src/examples/json/stability_check.json`. See [Running Harmony — Visual Studio](running-harmony.md#visual-studio).
