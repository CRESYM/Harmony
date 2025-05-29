# Installation Instructions

This document describes how to set up a development environment that allows you to compile and run Harmony.

## Prerequisites 
Harmony can be compiled on Windows, Linux (tested in Ubuntu 22.04.5), and macOS (tested in Sequoia 15.5). The requirements for each operating system are:
- Compilers for C and C++
    - Linux: GNU C and C++ compilers (tested version with 11.4.0)
    - MacOS: Apple Clang C and C++ compilers (tested with version 17.0.0)
    - Windows: [Visual Studio](https://visualstudio.microsoft.com) - version 2022 or newer, with a "Desktop Development for C++" workload installed (See [installation docs.](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022))
- [miniconda](https://www.anaconda.com/docs/getting-started/miniconda/main) - a miniature version of Anaconda that includes only conda, Python, and a few other packages. (You can also use Anaconda if it's already installed on your machine.)
- [Gurobi C++ API](https://docs.gurobi.com/projects/optimizer/en/current/reference/cpp.html): C++ API for optimization (license required).
- [Matplot++](https://alandefreitas.github.io/matplotplusplus/): C++ plotting library.

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

**1.** If you haven't yet done so, clone the Harmony repository by executing the commands: 
```bash
# Download repository
git clone https://github.com/CRESYM/Harmony.git
# Open the folder
cd Harmony
``` 
Alternatively, navigate to the Harmony repository using the `cd <path-to-harmony>` command, for example: `cd C:\Users\<netid>\Desktop\Harmony`

After opening the Harmony repository, use the `ls` command to verify you are in the correct directory. All of Harmony's files should be printed in the terminal when executing the command.

**2. Create the conda environment** with all of Harmony's dependencies. Creating the environment <ins>only needs to be done once</ins>. The file [environment.yml](../environment.yml) specifies the name the environment will be given and the libraries that will be installed within it. To create the environment, run the following command from the terminal: 
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
cmake ..
```

The VS Solution or makefile will be inside the `build` directory. The VS Solution or makefile needs to be regenerated every time you make changes to *CMakeLists.txt*. These changes can include, for example: adding a new *.h* or *.cpp* file to the project, adding a dependency on a third-party library, etc. To regenerate the solution or makefile, delete the `build` folder and re-run the commands listed in this step.

After building the harmony, two other libraries need to be installed and configured: `Gurobi` and `Matplot++`


**5. Matplot++ Installation using vcpkg**
`vcpkg` can be used to install `Matplot++` following these steps to install and configure the library:

Download `vcpkg`:
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.bat
   ```
   Intergrate`vcpkg` with Visual Studio:
   ```bash
   ./vcpkg integrate install
   ```
Install `Matplot++` via `vcpkg`. Open  `Powershell` or `CMD`, and navigate to the  `vcpkg` directory, like: 

```bash
   cd C:\XXX\vcpkg
```
   In the root directory of `vcpkg`, type:

   ```bash
   .\vcpkg install matplotplusplus
   ```
**6. Matplot++ Configuration in Visual Studio IDE**

After installing `Matplot++`, you probably still have problems running it successfully. 

First,  `Matplot++` only uses `C++17` or higher. In your Visual Studio, you need to go to `Configuration Properties` and then set:

<p align="left">
  <img src="assets\matplot++_f4.jpg" alt="Logo" width="660">
</p> 

Then, you will still get an error report:
   ```bash
   Unhandled exception at 0x0007ffe9ececf19 in MatplotPlusPlus_text.exe:
   Microsoft C++ exception: std::runtime_error at memory location 0x000000F632EFEC68.
   ```
This is because `Matplot++` relies on [gunplot](https://sourceforge.net/projects/gnuplot/) to achieve the plot function. You need to download and install `gunplot`.
<p align="left">
  <img src="assets\matplot++_f3.png" alt="Logo" width="660">
</p> 

After installing `gunplot`, add the `gunplot.exe` file to `PATH` under `System Variables` :
<p align="left">
  <img src="assets\matplot++_f2.jpg" alt="Logo" width="350">
</p> 

<p align="left">
  <img src="assets\matplot++_f1.jpg" alt="Logo" width="350">
</p> 

Last, add the `gunplot` library path in Visual Studio:
<p align="left">
  <img src="assets\matplot++_f5.jpg" alt="Logo" width="660">
</p> 

**7. Gurobi Configuration in Visual Studio IDE**

download [Gurobi](https://www.gurobi.com/downloads/) manually and install it follow a [Instruction](https://support.gurobi.com/hc/en-us/articles/4534161999889-How-do-I-install-Gurobi-Optimizer).
   

**8. Gurobi Configuration in Visual Studio IDE**
  
 Below are details regarding the Gurobi Configuration. Based on our local setup, the configuration is demonstrated using `Gurobi 9.5.2` and `Visual Studio 2022`.

Firstly, you will find two folders, `include` and `lib`, inside your Gurobi installation directory. 
  
  <p align="left">
  <img src="assets\gurobi_config_f1.png" alt="Logo" width="450">
  </p>  

  For example, if you installed `Gurobi` to `D:\gurobi952\win64`, then:
  
  - Header files are located at:  
  `D:\gurobi952\win64\include`
  <p align="left">
    <img src="assets\gurobi_config_f2.png" alt="Logo" width="450">
  </p> 

  - Library files are located at:  
  `D:\gurobi952\win64\lib`
  <p align="left">
    <img src="assets\gurobi_config_f3.png" alt="Logo" width="450">
  </p> 

Then, set `External Include Directories`:

Step1: right-click `project` ➡ select `Properties` ➡ go to `VC++ Drectories`
 
Step2: select `External Include Directories` ➡ add  `D:\gurobi952\win64\include` 

Step3: click `OK`


  <p align="left">
    <img src="assets\gurobi_config_f4.jpg" alt="Logo" width="660">
  </p> 

 Next, set `Additional Library Directories`:

 Step1: right-click `project` ➡ select `Properties` ➡ go to `Linker` ➡ select`General`
 
 Step2: select `Additional Library Directories` ➡ add  `D:\gurobi952\win64\lib` 

 Step3: click `OK`

<p align="left">
  <img src="assets\gurobi_config_f5.jpg" alt="Logo" width="660">
</p> 

 Last, add `Additional Dependencies`:

 Step1: right-click `project` ➡ select `Properties` ➡ go to`Linker` ➡ select`Input`
 
 Step2: select `Additional Dependencies` ➡ add  `D:\gurobi952\win64\lib\gurobi95.lib` and `D:\gurobi952\win64\lib\gurobi_c++mdd2019.lib` (for Debug), `D:\gurobi952\win64\lib\gurobi_c++md2019.lib` (for Release)

 Step3: click `OK`

<p align="left">
  <img src="assets\gurobi_config_f6.jpg" alt="Logo" width="660">
</p> 


**9.** (Optional, Windows) Open the VS Solution created in the previous step. Do so by selecting File->Open->Project/Solution on the top menu. Choose the file `<path-to-harmony/build/Harmony.sln>`. Carry out your development as usual, making changes to the project's source files and saving them.

Note: Harmony will add the ALL_BUILD and ZERO_CHECK projects to the VS solution. You can ignore them.

**10. Compile Harmony.** The following command (which needs to be run from the `build` folder containing the VS Solution or makefile) will build Harmony in Release mode. Use `--config Debug` to build in Debug mode.
```bash
cmake --build . --config Release
```

Windows users: You may also use the VS Build button or select Build->Solution from the menu at the top. The output file *Harmony.exe* will be created in the `build/Release` directory or `build/Debug` if the app was built in Debug mode.

**11. Execute Harmony.**
```bash
# Windows only: open the folder containing Harmony.exe
cd Release
# Run Harmony
./Harmony
```

Windows users: If you want to use the VS "Run" button or menu options, you need to set the Harmony project as the startup project in the Solution (CMake adds ALL_BUILD and ZERO_CHECK projects). To do so: right-click on the Harmony project in the *Solution Explorer* and select the option *Set as Startup Project*.
