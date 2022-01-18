cmake_minimum_required (VERSION 3.13)

add_executable(mvimporter_sandbox)
set_property(TARGET mvimporter_sandbox PROPERTY CXX_STANDARD 17)

target_compile_definitions(mvimporter_sandbox

	PRIVATE
		$<$<CONFIG:Debug>:MV_DEBUG>
		$<$<CONFIG:Release>:MV_RELEASE>
)

target_sources(mvimporter_sandbox

	PRIVATE

		"main.cpp"
		"mvSandbox.cpp"

		# core
		"src/core/mvViewport.cpp"
		"src/core/mvTimer.cpp"
		"src/core/mvMath.cpp"

		# renderer
		"src/renderer/mvAssetManager.cpp"
		"src/renderer/mvAssetLoader.cpp"
		"src/renderer/mvAnimation.cpp"
		"src/renderer/mvGraphics.cpp"
		"src/renderer/mvBuffers.cpp"
		"src/renderer/mvRenderer.cpp"
		"src/renderer/mvCamera.cpp"
		"src/renderer/mvMesh.cpp"
		"src/renderer/mvTextures.cpp"
		"src/renderer/mvPipeline.cpp"
		"src/renderer/mvMaterials.cpp"
		"src/renderer/mvLights.cpp"
		"src/renderer/mvEnvironment.cpp"

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

target_include_directories(mvimporter_sandbox

	PRIVATE

		"."
		".."
		"src/core/"
		"src/renderer/"

		"shaders/"
		"shaders/common/"

		"vendor/"
		"vendor/imgui/"
		"vendor/imgui/backends/"
		"vendor/stb/"

)

target_link_libraries(mvimporter_sandbox

	PRIVATE
		d3d11
		d3dcompiler
)