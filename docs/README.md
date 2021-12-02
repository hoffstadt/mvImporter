# mvImporter

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

    // now do whatever you want with the data

    // cleanup
    mvCleanupGLTF(gltfmodel);
}
```
