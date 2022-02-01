cmake_minimum_required (VERSION 3.13)

add_executable(sandbox_metal)
set_property(TARGET sandbox_metal PROPERTY CXX_STANDARD 17)


target_sources(sandbox_metal

	PRIVATE

		"main.mm"

		# core
		"src/core/mvFileIO.cpp"

		# renderer
		"src/renderer/mvGraphics.cpp"
		"src/renderer/mvPipeline.cpp"
		"src/renderer/mvRenderer.cpp"

		# semper
		"vendor/Semper/sSemper.mm"
		"vendor/Semper/sSemper_apple.mm"

		"vendor/Metal/Metal.cpp"
)

target_include_directories(sandbox_metal

	PRIVATE

		"."
		".."
		"src/core/"
		"src/renderer/"

		"vendor/"
		"../Sandbox/vendor/"
		"../Sandbox/vendor/Semper/"
		"../Sandbox/vendor/imgui/"
		"../Sandbox/vendor/imgui/backends/"
		"../Sandbox/vendor/stb/"
)

SET_PROPERTY(TARGET sandbox_metal APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")

set(CMAKE_EXE_LINKER_FLAGS "\
		-framework Metal \
		-framework MetalKit \
		-framework Cocoa \
		-framework IOKit \
		-framework CoreVideo \
		-framework QuartzCore")