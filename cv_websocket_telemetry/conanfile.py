from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class CvWsTelemetryConan(ConanFile):
    name = "cv_ws_telemetry"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("boost/1.85.0")
        self.requires("nlohmann_json/3.11.3")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def layout(self):
        self.folders.generators = "."
