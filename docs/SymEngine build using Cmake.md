### SymEngine installation in case of not using miniconda installation

1. Build GMP(GNU Multiple Precision Arithmetic Library): Ensure that GMP is built and installed correctly on your system. 
     - Download the GMP Source Code
     - Extract the Source Code 
     - Configure GMP: This bash script prepares the build environment and checks for any necessary dependencies. For x64 (64-bit):
     ```./configure --prefix=/path/to/installation/directory/of/gmp --host=x86_64-pc-msys```
     - make #to compile the library from source.
     - make install #install GMP to the specified installation directory 
     - make check #runs tests to ensure the library was built correctly and is functioning as expected.
     - Add to PATH: "System variables" (for all users) or "User variables" (just for your user), locate the PATH ...
         ```C:/Users/ur username/installation directory of gmp```

     * make sure that m4 is installed on your system (a macro processor typically used in the building of GNU packages), you can use MSYS2 shell to install m4, by the following scipts:
          - pacman -Syu  #to update the package database
          - pacman -S m4 #install m4 using pacman
          - m4 --version #verify installation

     * make sure that GCC and G++ cimpilers are installed in your MSYS environment using the following scripts
          - pacman -Syu #update package databases
          - pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gcc #install using the package manager for 64-bit architecture
          - need to update PATH environment variable by adding the following line in your '.bashrc' usually located in 'C:\msys64\home\ur username': ```export PATH=/mingw64/bin:$PATH``` 
          - gcc -v #verify installation

     * make sure that the GMP library is detected and linked correctly during the build process
          - Set GMP Include and Library Paths: Ensure that the include directory containing GMP headers (gmp.h) 
               ```$ export C_INCLUDE_PATH=$C_INCLUDE_PATH:/path/to/gmp/include/directory```

     * and the library directory containing GMP binaries (libgmp.a or libgmp.lib) are properly set in the build environment (where the symengine is located). 
     ```$ export LIBRARY_PATH=$LIBRARY_PATH:/path/to/gmp/lib/directory```
     
     This typically involves setting the C_INCLUDE_PATH (for headers) and LIBRARY_PATH (for libraries) environment variables or specifying these paths in your build system configuration.

2. Build MPFR
     - Download MPFR
     - Extract the source code
     - Create a directory for MPFR installation:
     ```mkdir new\folder\for\MPFR\installation```
     - Navigate to the MPFR Directory
     - ```./configure --prefix=path\to\install\directory --with-gmp=path\to\gmp```
     - Build and Install MPFR:
     ```
     make
     make check
     make install
     ```

3. Build SymEngine: Once GMP is built and installed, you can proceed to build SymEngine, ensuring that it is configured to use the GMP library.
     - Download SymEngine source code
     - Extract the Source Code
     - Navigate to the SymEngine Directory 
    ```
    cmake -DENABLE_GMP=ON /path/to/symengine/source/code #if you got warning please solve it by modifying CMakeList.txt file
    mkdir build 
    cd build
    cmake -DWITH_GMP=ON ..  #This command configures the build with GMP support enabled.
    cmake --build . --config Debug #build SymEngine using Visual studio
    ```
    
     * If the header files and CMake-related files are located in different directories within your project. You need to modify your CMakeLists.txt file to ensure that it can locate the header files in the correct directory
in CMakeList.txt after
```
     cmake_minimum_required(VERSION 3.0)
     project(YourProjectName)
```
add the following lines.

**Specify the directory where your header files are located**

```include_directories(header path)```

**Add your source files**
```add_executable(YourExecutableName source1.cpp source2.cpp)```

     - Clone SymEngine Repository
     - Open Visual Studio Solution: Navigate to the SymEngine directory and look for a Visual Studio solution file (.sln). 
     If there isn't one, you need to generate it using CMake. 
     - Navigate to symengine folder
     - mkdir build
     - cd build
     - cmake -G "Visual Studio 15 2017" -A x64 -DCMAKE_MODULE_PATH="path/to/cmake/where/FindGMP.cmake/exists" -     DGMP_INCLUDE_DIR="path/to/gmp/header/file" -DGMP_LIBRARY="path/to/gmp/lib/file" ..

     - Once you have the solution file, open it in Visual Studio.
     - Configure Build: If required, configure the build settings in Visual Studio according to your preferences. 
     - Build SymEngine: Build the SymEngine project in Visual Studio. 
     - Link to Your Project: Once SymEngine is built, you can link to it in your C++ project.
     You'll need to include the appropriate header files from SymEngine in your project and specify the location of the library files.
     - Configure Project Settings: In your C++ project settings, specify the include directories where SymEngine headers are located and link against the SymEngine library files.

4. Build Your Project: Finally, build your project, ensuring that it is linked correctly with both GMP and SymEngine libraries.
