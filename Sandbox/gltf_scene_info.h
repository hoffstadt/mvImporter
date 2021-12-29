#pragma once

#define MV_GLTF_MODELS \
    X(2CylinderEngine) \
    X(AlphaBlendModeTest) \
    X(AnimatedCube) \
    X(AnimatedMorphCube) \
    X(AnimatedMorphSphere) \
    X(AnimatedTriangle) \
    X(AntiqueCamera) \
    X(AttenuationTest) \
    X(Avocado) \
    X(BarramundiFish) \
    X(BoomBox) \
    X(BoomBoxWithAxes) \
    X(Box) \
    X(Box With Spaces) \
    X(BoxAnimated) \
    X(BoxInterleaved) \
    X(BoxTextured) \
    X(BoxTexturedNonPowerOfTwo) \
    X(BoxVertexColors) \
    X(BrainStem) \
    X(Buggy) \
    X(Cameras) \
    X(CesiumMan) \
    X(CesiumMilkTruck) \
    X(ClearCoatTest) \
    X(Corset) \
    X(Cube) \
    X(DamagedHelmet) \
    X(DragonAttenuation) \
    X(Duck) \
    X(EnvironmentTest) \
    X(FlightHelmet) \
    X(Fox) \
    X(GearboxAssy) \
    X(GlamVelvetSofa) \
    X(InterpolationTest) \
    X(IridescentDishWithOlives) \
    X(Lantern) \
    X(MaterialsVariantsShoe) \
    X(MetalRoughSpheres) \
    X(MetalRoughSpheresNoTextures) \
    X(MorphPrimitivesTest) \
    X(MorphStressTest) \
    X(MosquitoInAmber) \
    X(MultiUVTest) \
    X(NormalTangentMirrorTest) \
    X(NormalTangentTest) \
    X(OrientationTest) \
    X(ReciprocatingSaw) \
    X(RecursiveSkeletons) \
    X(RiggedFigure) \
    X(RiggedSimple) \
    X(SciFiHelmet) \
    X(SheenChair) \
    X(SheenCloth) \
    X(SimpleMeshes) \
    X(SimpleMorph) \
    X(SimpleSkin) \
    X(SimpleSparseAccessor) \
    X(SpecGlossVsMetalRough) \
    X(SpecularTest) \
    X(Sponza) \
    X(StainedGlassLamp) \
    X(Suzanne) \
    X(TextureCoordinateTest) \
    X(TextureEncodingTest) \
    X(TextureLinearInterpolationTest) \
    X(TextureSettingsTest) \
    X(TextureTransformMultiTest) \
    X(TextureTransformTest) \
    X(ToyCar) \
    X(TransmissionRoughnessTest) \
    X(TransmissionTest) \
    X(Triangle) \
    X(TriangleWithoutIndices) \
    X(TwoSidedPlane) \
    X(UnlitTest) \
    X(VC) \
    X(VertexColorTest) \
    X(WaterBottle)

#define MV_GLTF_EMBEDDED_MODELS \
    Y(2CylinderEngine) \
    Y(AlphaBlendModeTest) \
    Y(AnimatedTriangle) \
    Y(Box) \
    Y(BoxAnimated) \
    Y(BoxInterleaved) \
    Y(BoxTextured) \
    Y(BoxTexturedNonPowerOfTwo) \
    Y(BoxVertexColors) \
    Y(BrainStem) \
    Y(Buggy) \
    Y(Cameras) \
    Y(CesiumMan) \
    Y(CesiumMilkTruck) \
    Y(DamagedHelmet) \
    Y(Duck) \
    Y(Fox) \
    Y(GearboxAssy) \
    Y(MetalRoughSpheres) \
    Y(MultiUVTest) \
    Y(NormalTangentMirrorTest) \
    Y(NormalTangentTest) \
    Y(OrientationTest) \
    Y(ReciprocatingSaw) \
    Y(RiggedFigure) \
    Y(RiggedSimple) \
    Y(SimpleMeshes) \
    Y(SimpleMorph) \
    Y(SimpleSkin) \
    Y(SimpleSparseAccessor) \
    Y(TextureCoordinateTest) \
    Y(TextureSettingsTest) \
    Y(Triangle) \
    Y(TriangleWithoutIndices) \
    Y(VC) \
    Y(VertexColorTest)

#define MV_GLTF_BINARY_MODELS \
    Z(2CylinderEngine) \
    Z(AlphaBlendModeTest) \
    Z(AnimatedMorphCube) \
    Z(AnimatedMorphSphere) \
    Z(AntiqueCamera) \
    Z(AttenuationTest) \
    Z(Avocado) \
    Z(BarramundiFish) \
    Z(BoomBox) \
    Z(Box) \
    Z(BoxAnimated) \
    Z(BoxInterleaved) \
    Z(BoxTextured) \
    Z(BoxTexturedNonPowerOfTwo) \
    Z(BoxVertexColors) \
    Z(BrainStem) \
    Z(Buggy) \
    Z(CesiumMan) \
    Z(CesiumMilkTruck) \
    Z(ClearCoatTest) \
    Z(Corset) \
    Z(DamagedHelmet) \
    Z(DragonAttenuation) \
    Z(Duck) \
    Z(Fox) \
    Z(GearboxAssy) \
    Z(GlamVelvetSofa) \
    Z(InterpolationTest) \
    Z(IridescentDishWithOlives) \
    Z(Lantern) \
    Z(MaterialsVariantsShoe) \
    Z(MetalRoughSpheres) \
    Z(MetalRoughSpheresNoTextures) \
    Z(MorphPrimitivesTest) \
    Z(MorphStressTest) \
    Z(MosquitoInAmber) \
    Z(MultiUVTest) \
    Z(NormalTangentMirrorTest) \
    Z(NormalTangentTest) \
    Z(OrientationTest) \
    Z(ReciprocatingSaw) \
    Z(RecursiveSkeletons) \
    Z(RiggedFigure) \
    Z(RiggedSimple) \
    Z(SheenChair) \
    Z(SpecGlossVsMetalRough) \
    Z(SpecularTest) \
    Z(TextureCoordinateTest) \
    Z(TextureEncodingTest) \
    Z(TextureLinearInterpolationTest) \
    Z(TextureSettingsTest) \
    Z(TextureTransformMultiTest) \
    Z(ToyCar) \
    Z(TransmissionRoughnessTest) \
    Z(TransmissionTest) \
    Z(UnlitTest) \
    Z(VC) \
    Z(VertexColorTest) \
    Z(WaterBottle)

#define X(el) "../../data/glTF-Sample-Models/2.0/" ## #el ## "/glTF/" ## #el ## ".gltf" ## ,
#define Y(el) "../../data/glTF-Sample-Models/2.0/" ## #el ## "/glTF-Embedded/" ## #el ## ".gltf" ## ,
#define Z(el) "../../data/glTF-Sample-Models/2.0/" ## #el ## "/glTF-Binary/" ## #el ## ".glb" ## ,
static const char* gltf_models[] = {
    MV_GLTF_MODELS
    MV_GLTF_EMBEDDED_MODELS
    MV_GLTF_BINARY_MODELS
};
#undef X
#undef Y
#undef Z

#define X(el) "../../data/glTF-Sample-Models/2.0/" ## #el ## "/glTF/" ## ,
#define Y(el) "../../data/glTF-Sample-Models/2.0/" ## #el ## "/glTF-Embedded/" ## ,
#define Z(el) "../../data/glTF-Sample-Models/2.0/" ## #el ## "/glTF-Binary/" ## ,
static const char* gltf_directories[] = {
    MV_GLTF_MODELS
    MV_GLTF_EMBEDDED_MODELS
    MV_GLTF_BINARY_MODELS
};
#undef X
#undef Y
#undef Z

#define X(el) #el ## ".gltf" ## ,
#define Y(el) #el ## ".gltf (embedded)" ## ,
#define Z(el) #el ## ".glb (binary)" ## ,
static const char* gltf_names[] = {
    MV_GLTF_MODELS
    MV_GLTF_EMBEDDED_MODELS
    MV_GLTF_BINARY_MODELS
};
#undef X
#undef Y
#undef Z

static const char* env_maps[] = {
    "none",
    "Cannon_Exterior",
    "Cannon_Studio",
    "chromatic",
    "directional",
    "doge2",
    "ennis",
    "field",
    "footprint_court",
    "helipad",
    "neutral",
    "papermill",
    "pisa",
    "Wide_Street",
};