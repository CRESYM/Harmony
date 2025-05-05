# Installation Instructions

This document describes how to setup a development environment that allows you to compile and run Harmony.

## Prerequisites 
Harmony is currently only available on Windows.
- [Visual Studio](https://visualstudio.microsoft.com) - version 2022 or newer, with a "Desktop Development for C++" workload installed (See [installation docs.](https://learn.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2022))
- [miniconda](https://www.anaconda.com/docs/getting-started/miniconda/main) - a miniature version of Anaconda that includes only conda, Python and other few packages. (You can also use Anaconda if it's already installed on your machine.)

> [!WARNING]
> When installing miniconda, make sure you select the option **Add to path**. This will allow you to use miniconda from Visual Studio, Git Bash, etc. If you forgot this, have a look at Step 3 in [this blog post](https://eduand-alvarez.medium.com/setting-up-anaconda-on-your-windows-pc-6e39800c1afb), which shows how to add miniconda to your path. Alternatively, you may reinstall miniconda.

<details>

<summary>Verify miniconda installation</summary>

Open Visual Studio. Close the Welcome dialogue or select the option *Continue without code*. Open the VS built-in terminal by selecting View->Terminal on the top menu. The terminal should display `(base)` at the left of the prompt symbol as shown in the picture below. This symbol indicates that VS can find and use miniconda. The word "base" means that no conda environments are activated; if an environment were activated, its name would appear instead of "base".

![Screenshot 2025-05-05 at 21 03 07](https://github.com/user-attachments/assets/f519969a-9976-4255-b226-6aa5a976f190)

**Troubleshooting**

If `(base)` is not displayed, attempt any of the following:
- Close and reopen Visual Studio. 
- With Visual Studio closed, open the Windows PowerShell in admin mode. To do so: click on the Windows button in the toolbar, look for Windows PowerShell in the list of installed apps, right-click on it an select *More -> Run as Administrator*. Enter your username and password to validate your admin priveleges. Run the following command, close the PowerShell and re-open Visual Studio:
```
Set-ExecutionPolicy RemoteSigned
```
</details>


## Installation

**1. Open Visual Studio.** Close the Welcome dialogue or select the option *Continue without code*.

**2.** Open the Visual Studio built-in terminal by selecting View->Terminal on the top menu.

**3.** If you haven't yet done so, clone the Harmony repository by executing the commands: 
```bash
# Download repository
git clone https://github.com/CRESYM/Harmony.git
# Open the folder
cd Harmomy
``` 
Alternatively, navigate to the Harmony repository using the `cd <path-to-harmony>` command, for example: `cd C:\Users\<netid>\Desktop\Harmony`

After opening the Harmony repository, use the `ls` command to verify you are in the correct directory. All of Harmony's files should be printed in the terminal when executing the command.

**4. Create the conda environment** with all of Harmony's dependencies. Creating the environment <ins>only needs to be done once</ins>. The file [environment.yml](../environment.yml) specifies the name the environment will be given and the libraries that will be installed within it. To create the environment, run the following command from the VS terminal: 
``` bash
# Create the harmony conda environment (done once)
conda env create -f environment.yml
```

The `conda env list` command prints the names of the conda environments installed on your machine. You can use it to verify the harmony environment was created successfuly. See [Managing Conda Environments](https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html) for guidance on how to work with conda environments.


**5. Activate the conda environment** created in the previous step. You will need to activate the environment everytime you want to compile or execute Harmony. Activating the environment allows Visual Studio to find Harmony's dependencies. To activate it, run: 
``` bash
# Activate the harmony conda environment (done every time you want to compile or run harmony)
conda activate harmony
```

If the environment was correctly activated, `(harmony)` should be displayed to the left of the terminal's prompt. Use `conda deactivate` to deactivate the harmony environment - this will replace `(harmony)` with `(base)` on the terminal's prompt. 

![Screenshot 2025-05-05 at 21 14 39](https://github.com/user-attachments/assets/2c430c03-0438-4afa-95f9-75d697d7642a)

> [!TIP]
> Always make sure `(harmony)` is displayed on the VS terminal prompt before attempting to compile or run harmony. This symbol indicates the environment is activated.

**6. Use CMake** to create the Harmony Visual Studio Solution. The file [CMakeLists.txt](../CMakeLists.txt) contains the instructions to create the VS Solution.
```bash
# Create a build directory where the VS Solution will be generated
mkdir build
# Open the build directory
cd build
# Generate the VS Solution. CMake will look for CMakeLists.txt in the parent directory of the build folder (indicated by ..)
cmake ..
```

The Solution will be inside the `build` directory. The VS Solution needs to be regenerated every time you make changes to *CMakeLists.txt*. These changes can include, for example: adding a new *.h* or *.cpp* file to the project, adding a dependency on a third party library, etc. To regenerate the VS Solution, delete the `build` folder that contains it, and re-run the commands listed in this step.

**7.** (Optional) Open the VS Solution created in the previous step. Do so by selection File->Open->Project/Solution on the top menu. Choose the file `<path-to-harmony/build/Harmony.sln>`. Carry out your development as per usual, making changes to the project's source files and saving them.

Note: Harmony will add the ALL_BUILD and ZERO_CHECK projects to the solution. You can ignore them.

**8. Compile Harmony.** The following command (which needs to be run from the `build` folder containing the VS Solution) will build Harmony in Release mode. Use `--config Debug` to build in Debug mode.
```bash
cmake --build . --config Release
```

Alternatively, you may use the VS Build button or select Build->Solution from the menu at the top. The output file *Harmony.exe* will be created in the `build/Release` directory or `build/Debug` if the app was built in in Debug mode.

**9. Execute Harmomy.**
```bash
# Open the folder containing Harmony.exe
cd Release
# Run Harmony
./Harmony.exe
```

If you want to use the VS "Run" button or menu options, you need to set the Harmony project as the startup project in the Solution (CMake adds ALL_BUILD and ZERO_CHECK projects). To do so: right-click on the Harmony project in the *Solution Explorer* and select the option *Set as Startup Project*.
