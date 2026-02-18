This package contains (almost) everything you need to start making OpenGL programs. You just need the following additional things:

1. A C++ compiler. Instructions for installing the compiler are provided in the latter part of this README.md file.

   - For Linux (Ubuntu variants, including WSL), we shall use the standard gcc compiler.
   - For Windows, we shall use the MinGW-w64 gcc compiler.
   - For macOS, we shall use the clang compiler.

2. A text editor. I recommend that you use Visual Studio Code (although you can use any editor you prefer). If you do install VS Code:

   - I recommend installing the following extensions (click View->Extensions and search for these in the Marketplace):

     - ms-vscode.cpptools (C/C++ IntelliSense)
     - slevesque.shader (Shader languages support for VS Code)

   - You can open VS Code from a Terminal window at any time by typing:

         code .

     Note the space and the period at the end. This refers to the current directory of your terminal. This ensures that you are editing files from the correct directory! (I've seen too many students editing one file but compiling another, and wondering why they still have errors...)

Happy hacking! - eric

*******************************************************************************

Instructions for Linux (Ubuntu variants, including WSL):
--------------------------------------------------------

0. [If you are actually using Windows] I recommend installing the Windows Subsystem for Linux (WSL). Simply open a Command Prompt or Windows Powershell window. (From the Start menu, type the word "cmd" or "powershell", and click the relevant item that appears.) Then enter the command:

       wsl --install

   Follow the instructions to install the default distribution (Ubuntu LTS).
   - You may need to reboot, and you may need to enter the above command again to finish installation.
   - You will be prompted to create a username and password for your Ubuntu installation. This is different from your Windows local account. Remember your password for the next step.

1. Open a new Ubuntu Terminal window, then enter the following commands, one line at a time (you may be prompted for your password):

       sudo apt update && sudo apt upgrade
       sudo apt install zip unzip build-essential libglfw3-dev

   - The first line installs essential updates to your Ubuntu distribution. The second line installs the command line tools and glfw development libraries that we need for C++ and OpenGL development.
   - Each of these steps may take a couple of hours (depending on your connection speed).

2. Confirm that your compiler is working by entering the command:

       g++

   The command should give you the message:

       g++: fatal error: no input files
       compilation terminated.

3. Extract the gdevgl.zip file into your Ubuntu home directory using the unzip command. (If you are using WSL, do not simply copy the files over using Windows Explorer because file ownership will not be set correctly.)

   For example, if you downloaded gdevgl.zip to your Windows local account's Downloads folder (in the case of a WSL installation), enter the following commands, one line at a time:

       mkdir ~/gdevgl
       cd ~/gdevgl
       unzip /mnt/c/Users/{your Windows username}/Downloads/gdevgl.zip

   Replace the path after the unzip command with the path to where gdevgl.zip actually resides.

4. A testing script is provided for you (so that you don't have to type lengthy compilation commands when you test your OpenGL programs), but it will not work out-of-the-box. Type this command to make it work (you only need to do this once):

       chmod +x ./test

5. Finally, to try out one of the example programs, run the testing script. For example, to test out the first demo, type:

       ./test demo0.cpp

   The test script will helpfully display the full command line that you would actually type in to manually compile your program. (Aren't you glad that we can automate this?) The command line may also serve as a guide for when you make your own "makefiles" to build larger applications.

*******************************************************************************

Instructions for Windows:
-------------------------

1. Download and install the MinGW-w64 gcc compiler.

   - I recommend that you download precompiled binaries from https://github.com/niXman/mingw-builds-binaries/releases/download/15.2.0-rt_v13-rev0/x86_64-15.2.0-release-posix-seh-ucrt-rt_v13-rev0.7z . Otherwise, you can figure out how to build it yourself from https://www.mingw-w64.org/ (not recommended for this class).

   - Extract the contents of the .7z file to your C:\ drive. If you cannot open the file (e.g., on earlier versions of Windows that doesn't recognize .7z files), try https://www.7-zip.org/.

   - If you did it correctly, there should be a mingw64 folder (the contents of the .7z) directly on your C:\ drive.

2. Add the folder C:\mingw64\bin to your system's environment variables.

   - Open the Start menu, type the word "env", then click "Edit environment variables for your account".

     - If you instead clicked "Edit the system environment variables": On the window that appears, click on the "Environment Variables..." button near the bottom-right.

   - On the upper pane of the Environment Variables window, double-click the variable "Path".

   - Click "New", then type in "C:\mingw64\bin" (without the quotes).

   - Click OK to confirm the addition.

   - Click OK (on the Environment Variables window) to save the environment variables.

3. Open a Command Prompt or Windows Powershell window. (From the Start menu, type the word "cmd" or "powershell", and click the relevant item that appears.)

4. Confirm that your MinGW-w64 compiler is working by entering the command:

       g++

   The command should give you the message:

       g++: fatal error: no input files
       compilation terminated.

5. Change to the folder where you extracted this gdevgl.zip.

   For example, if you extracted it to your Windows desktop as-is, type the command:

       cd C:\Users\{your username}\Desktop\gdevgl

6. Finally, to try out one of the example programs, run the test.cmd script provided. For example, to test out the first demo, type:

       .\test demo0.cpp

   The test script will helpfully display the full command line that you would actually type in to manually compile your program. (Aren't you glad that we can automate this?) The command line may also serve as a guide for when you make your own "makefiles" to build larger applications.

*******************************************************************************

Instructions for macOS:
-----------------------

1. Open a Terminal window. (Click the Launchpad icon on the Dock, then click "Other", then "Terminal".)

2. Confirm whether you have the command line developer tools installed by entering the command:

       clang++

   - If you have not used clang or Xcode before, this will prompt you to install the command line developer tools. Click "Install", and wait for a couple of hours (depending on your connection speed).

   - If it is already installed, the command should give you the message:

         clang: error: no input files

3. Extract the gdevgl.zip file into your home directory using the unzip command.

   For example, if you downloaded gdevgl.zip to your macOS local account's Downloads folder, enter the following commands, one line at a time:

       mkdir ~/gdevgl
       cd ~/gdevgl
       unzip ~/Downloads/gdevgl.zip

   Replace the path after the unzip command with the path to where gdevgl.zip actually resides.

4. A testing script is provided for you (so that you don't have to type lengthy compilation commands when you test your OpenGL programs), but it will not work out-of-the-box. Type this command to make it work (you only need to do this once):

       chmod +x ./test

5. Finally, to try out one of the example programs, run the testing script. For example, to test out the first demo, type:

       ./test demo0.cpp

   The test script will helpfully display the full command line that you would actually type in to manually compile your program. (Aren't you glad that we can automate this?) The command line may also serve as a guide for when you make your own "makefiles" to build larger applications.

*******************************************************************************

This package contains the following third-party software:
---------------------------------------------------------

1. GLAD version 0.1.36 (https://glad.dav1d.de/)

   - Automatically-generated header and library-loader files for OpenGL (since the ones that ship with Windows and macOS are cumbersome to use).

   - The GLAD in this package is generated for the gl API, Version 3.3, Core (add all extensions); also, the glad.c file is renamed to glad.cpp to avoid compilation warnings, since we will be using C++.

2. GLFW version 3.4 (https://www.glfw.org/download.html)

   - A cross-platform wrapper library for OpenGL that provides auxiliary OS functions such as window system management and input handling.

   - Only the header files (in the include/ subdirectory) and the Windows (lib-mingw-w64/) and macOS (lib-universal/) precompiled library binaries are included in this package.

3. GLM version 1.0.3 (https://github.com/g-truc/glm/releases)

   - A C++ header-only library for math operations, closely following GLSL naming conventions and functionality.

   - Only the header files (in the glm/ subdirectory) are included in this package.

4. stb_image.h version 2.30 (https://github.com/nothings/stb/blob/master/stb_image.h)

   - A header-only library for loading images.
