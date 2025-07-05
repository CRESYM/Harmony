### Install SymEngine in your C++ Visual Studio project using vcpkg

1.Install vcpkg:
```
git clone https://github.com/microsoft/vcpkg.git (Clone the vcpkg repository)
cd vcpkg
.\bootstrap-vcpkg.bat
```

2. Integrate vcpkg with Visual Studio:
 ```.\vcpkg integrate install```

3. Install SymEngine: 
```.\vcpkg install symengine```

4. Link SymEngine to Your Project:

Follow these steps:

  - Open the Visual Studio project.
    Right-click on your project in the Solution Explorer and select "Properties".
    In the Properties window, go to Configuration Properties > VC++ Directories.

  - Add the include directory from vcpkg to Include Directories:
     ```[vcpkg-root]\packages\symengine\include```
     for example: ```D:\Softwares\vcpkg\packages\symengine_x64-windows\include\symengine;$(IncludePath)```

  - Add the library directory from vcpkg to Library Directories:
    ```
    [vcpkg-root]\packages\symengine\lib
     D:\Softwares:\vcpkg\packages\symengine_x64-windows\lib;$(LibraryPath)
    ```

  - Configuration Properties > C/C++ > General, add additional include directories: ```[vcpkg-   root]\packages\symengine\include``` for example: ```D:\Softwares\vcpkg\packages\symengine_x64-windows\include\symengine;$(IncludePath)```

   - Configuration Properties > Linker > General
     ```
      Additional Library Directories 
      [vcpkg-root]\packages\symengine\lib
      D:\Softwares:\vcpkg\packages\symengine_x64-windows\lib;$(LibraryPath)
     ```

5. Add SymEngine to Your Project: Configuration Properties > Linker > Input and then
Add symengine.lib to Additional Dependencies

6. Include SymEngine Headers in Your Code
```Cpp
#include <symengine/symbol.h>
#include <symengine/basic.h>
#include <symengine/add.h>
```

7. Build Your Project


