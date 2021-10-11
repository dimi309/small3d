from conans import ConanFile, CMake, tools
import os
import shutil

class Small3dConan(ConanFile):
    name = "small3d"
    version = "master"
    license = "BSD 3-Clause"
    author = "dimi309"
    url = "https://github.com/dimi309/small3d-conan"
    description = "Minimalistic, open source library for making 3D games in C++, with Vulkan and OpenGL support"
    topics = ("small3d", "opengl", "vulkan", "gamedev")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch", "cppstd"
    options = {"shared": [True, False], "fPIC": [True, False], "vulkan": [True, False]}
    default_options = {"shared": False, "fPIC": True, "vulkan": False}
    requires = "bzip2/1.0.8", "freetype/2.11.0", "glfw/3.3.4", "glm/0.9.9.8", "libpng/1.6.37", "vorbis/1.3.7", "zlib/1.2.11", "portaudio/19.7.0@bincrafters/stable"
    generators = "cmake"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src.CMakeLists.txt", "../src*", "../include*", "../scripts*", "../resources*", "../opengl33*", "LICENSE"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        if self.options.vulkan:
            self.requires("vulkan-loader/1.2.190")
            self.requires("vulkan-headers/1.2.190")
        else:
            self.requires("glew/2.2.0")

    def source(self):
        shutil.copy("src.CMakeLists.txt", "src/CMakeLists.txt")

    def build(self):
        cmake = CMake(self)
        if not self.options.vulkan:
            cmake.definitions["SMALL3D_OPENGL"] = True
        cmake.configure()
        cmake.build()
        if self.options.vulkan:
            if self.settings.build_type == "Release":
                if self.settings.os == "Windows":
                    self.run("compile-shaders.bat Release", "scripts", msys_mingw=False)
                else:
                    self.run("./compile-shaders.sh Release", "scripts", msys_mingw=False)
            else:
                if self.settings.os == "Windows":
                    self.run("compile-shaders.bat Debug", "scripts", msys_mingw=False)
                else:
                    self.run("./compile-shaders.sh Debug", "scripts", msys_mingw=False)

    def package(self):
        if self.options.vulkan:
            self.copy("*.spv", dst="resources", src="resources")
            self.copy("*.hpp", dst="include", src="include")
        else:
            self.copy("*", dst="resources/shaders", src="opengl33/resources/shaders")
            self.copy("*.hpp", dst="include", src="include", excludes="Renderer.*p")
            self.copy("*.hpp", dst="include", src="opengl33/include")
        
    
        self.copy(pattern="LICENSE*", dst="licenses", src="",  ignore_case=True, keep_path=False)
        self.copy(pattern="*.lib", dst="lib", keep_path=False)
        self.copy(pattern="*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["small3d"]
        if not self.options.vulkan:
            self.cpp_info.defines = ["SMALL3D_OPENGL"]
        if self.settings.os == "Windows" and self.settings.compiler == "gcc" and not self.options.shared:
            self.cpp_info.system_libs.append("setupapi") # This should normally be in the portaudio package

