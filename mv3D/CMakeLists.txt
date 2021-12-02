cmake_minimum_required (VERSION 3.13)

add_library(mv3d)
set_property(TARGET mv3d PROPERTY CXX_STANDARD 17)

target_compile_definitions(mv3d

	PRIVATE
		$<$<CONFIG:Debug>:MV_DEBUG>
		$<$<CONFIG:Release>:MV_RELEASE>
		CHARLIE
)

target_sources(mv3d

	PRIVATE

		"mv3D_internal.cpp"

		# core
		"src/core/mvViewport.cpp"
		"src/core/mvTimer.cpp"
		"src/core/mvMath.cpp"

		# renderer
		"src/renderer/mvAssetManager.cpp"
		"src/renderer/mvGraphics.cpp"
		"src/renderer/mvBuffers.cpp"
		"src/renderer/mvRenderer.cpp"
		"src/renderer/mvCamera.cpp"
		"src/renderer/mvVertexLayout.cpp"
		"src/renderer/mvMesh.cpp"
		"src/renderer/mvTextures.cpp"
		"src/renderer/mvPipeline.cpp"
		"src/renderer/mvMaterials.cpp"
		"src/renderer/mvLights.cpp"
		"src/renderer/mvScene.cpp"
		"src/renderer/mvShadows.cpp"
		"src/renderer/mvObjLoader.cpp"

		# implot
		"vendor/implot/implot.cpp"
		"vendor/implot/implot_demo.cpp"
		"vendor/implot/implot_items.cpp"

		# imgui
		"vendor/imgui/misc/cpp/imgui_stdlib.cpp"
		"vendor/imgui/imgui.cpp"
		"vendor/imgui/imgui_demo.cpp"
		"vendor/imgui/imgui_draw.cpp"
		"vendor/imgui/imgui_widgets.cpp"
		"vendor/imgui/imgui_tables.cpp"
		"vendor/imgui/backends/imgui_impl_win32.cpp"
		"vendor/imgui/backends/imgui_impl_dx11.cpp"
)

target_include_directories(mv3d

	PRIVATE

		"."
		"src/core/"
		"src/renderer/"

		"shaders/"
		"shaders/common/"

		"vendor/"
		"vendor/implot/"
		"vendor/imgui/"
		"vendor/imgui/backends/"
		"vendor/stb/"

)

target_link_libraries(mv3d

	PRIVATE
		d3d11
		d3dcompiler
)