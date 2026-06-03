from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout


class BallisticEngineConan(ConanFile):
    name = "ballistic-engine"
    version = "1.0.0"
    license = "Proprietary"
    description = "C++17 ballistic trajectory engine with RK4 integrator"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "include/*", "src/*"

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ballistic_engine"]
        self.cpp_info.set_property("cmake_target_name", "ballistic::engine")
