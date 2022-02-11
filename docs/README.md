# mvImporter (WIP)
A simple, portable header-only [glTF 2.0](https://github.com/KhronosGroup/glTF) loader with no dependencies. Similar to [TinyGLTF](https://github.com/syoyo/tinygltf) but with a built-in JSON parser.

| [Basic Usage](#basic-usage) | [Features](#features) | [TODOs](#todos) | [Licenses](#licenses) | [Sample Renderers](#sample-renderers) |
|-----------------------------|-----------------------|-----------------|-----------------------|---------------------------------------|

<div align="center">
  <img src="https://github.com/hoffstadt/mvImporter/blob/master/docs/images/damagedhelmet.png?raw=true" alt="DamagedHelmet">
</div>

## Basic Usage

```cpp

#define MV_IMPORTER_IMPLEMENTATION
#include "mvImporter.h"

int main()
{
    mvGLTFModel gltfmodel = mvLoadGLTF("C/dev/Resources/FlightHelmet/", "C:/dev/Resources/FlightHelmet/FlightHelmet.gltf");

    // counts
    unsigned int scene_count = gltfmodel.scene_count;
    unsigned int node_count = gltfmodel.node_count;
    unsigned int mesh_count = gltfmodel.mesh_count;
    unsigned int material_count = gltfmodel.material_count;
    unsigned int texture_count = gltfmodel.texture_count;
    unsigned int sampler_count = gltfmodel.sampler_count;
    unsigned int image_count = gltfmodel.image_count;
    unsigned int buffer_count = gltfmodel.buffer_count;
    unsigned int bufferview_count = gltfmodel.bufferview_count;
    unsigned int accessor_count = gltfmodel.accessor_count;
    unsigned int camera_count = gltfmodel.camera_count;

    // arrays
    mvGLTFScene* scenes = gltfmodel.scenes;
    mvGLTFNode* nodes = gltfmodel.nodes;
    mvGLTFMesh* meshes = gltfmodel.meshes;
    mvGLTFMaterial* materials = gltfmodel.materials;
    mvGLTFTexture* textures = gltfmodel.textures;
    mvGLTFSampler* samplers = gltfmodel.samplers;
    mvGLTFImage* images = gltfmodel.images;
    mvGLTFBuffer* buffers = gltfmodel.buffers; // note the data already loaded as bytes in the "data" member
    mvGLTFBufferView* bufferviews = gltfmodel.bufferviews;
    mvGLTFAccessor* accessors = gltfmodel.accessors;
    mvGLTFCamera* cameras = gltfmodel.cameras;

    // now do whatever you want with the data

    // cleanup
    mvCleanupGLTF(gltfmodel);
}
```

## Features

* Written in portable C++. C++-11 with STL dependency only.
  * [x] Windows + Visual Studio 2015 Update 3 or later.
  * [x] macOS + clang(LLVM)
  * [x] Linux + gcc/clang
  * [x] Windows + MinGW
* glTF specification v2.0.0
  * [x] ASCII glTF
    * [x] Load
  * [x] Binary glTF(GLB)
    * [x] Load
* Buffers
  * [x] Parse BASE64 encoded embedded buffer data(DataURI).
  * [x] Load `.bin` file.
* Image
  * [x] Parse BASE64 encoded embedded image data(DataURI)
  * [x] Load external image file(Using stb_image)
  * [x] Load PNG(8bit and 16bit)(Using stb_image)
  * [x] Load JPEG(8bit only)(Using stb_image)
  * [x] Load BMP(Using stb_image)
  * [x] Load GIF(Using stb_image)
* Morph traget
  * [ ] Sparse accessor
* Load glTF from memory
* Custom callback handler
  * [ ] Image load
  * [ ] Image save
* Extensions
  * [ ] Draco mesh decoding
  * [ ] Draco mesh encoding

## TODOs
* [ ] Remove C++ std dependencies on _string_.
* [ ] Add tests
* [ ] Support extensions and extras
* [x] Support animations
* [ ] Added toy engine for Linux

## Sample Renderers
We've included simple renderers to test loading and features. A lot of work still needs to be done here and they are not ready for general usage but feel free to play with them.
With the goal of this project being the loader, not the renderers, we only work on them when we have time.

### Building
#### Windows
**Requirements**
- [_git_](https://git-scm.com/)
- [_Visual Studio 2019_ (windows)](https://visualstudio.microsoft.com/vs/) with the following workflows:
  * Desktop development with C++
  * Game development with C++

Instructions:
1. From within a local directory, enter the following bash commands:
```
git clone --recursive https://github.com/hoffstadt/mvImporter
```
4. In Visual Studio, use _Open a local folder_ to open _mvImporter_ folder. This will use CMakeLists.txt as the "project" file.
5. Run _Build All_.

### Linux
Not ready yet.

### MacOS
WIP

## Licenses
_mvImporter_ is licensed under MIT license.

Battle Damaged Sci-fi Helmet - PBR by [theblueturtle_](https://sketchfab.com/theblueturtle_), published under a Creative Commons Attribution-NonCommercial license
