## Charlie

[![build status](https://github.com/hoffstadt/mv3D/workflows/Windows%20Builds/badge.svg?branch=master)](https://github.com/hoffstadt/mv3D/actions?workflow=Windows%20Builds)

A **Directx 11** and **Directx 12** 3D Engine.

## Building
### Windows
**Requirements**
- [_git_](https://git-scm.com/)
- [_Visual Studio 2019_ (windows)](https://visualstudio.microsoft.com/vs/) with the following workflows:
  * Python development
  * Desktop development with C++
  * Game development with C++

Instructions:
1. From within a local directory, enter the following bash commands:
```
git clone --recursive https://github.com/hoffstadt/mv3D
```
4. In Visual Studio, use _Open a local folder_ to open _mv3D_ folder. This will use CMakeLists.txt as the project file.
5. Run _Build All_.
