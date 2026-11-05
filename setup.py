from setuptools import Extension, setup

extension = Extension(
    name="tinyvm._native",
    sources=[
        "src/python/module.c",
        "src/core/value.c",
        "src/core/stack.c",
        "src/core/vm.c",
    ],
    include_dirs=["src/core"],
)

setup(
    packages=["tinyvm"],
    ext_modules=[extension],
)