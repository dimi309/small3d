from conans import ConanFile, CMake, tools
import os

class Small3dTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    options = {"vulkan": [True, False]}
    default_options = {"vulkan": False}
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.spv", dst="", src="")

    def test(self):
        if not tools.cross_building(self):
            if self.settings.os == "Windows":
                cmd = "bin\example"
            else:
                cmd = "bin/example"
            self.run(cmd, env="conanrunenv")
