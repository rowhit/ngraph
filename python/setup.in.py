# ******************************************************************************
# Copyright 2017-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ******************************************************************************

# ----------------------------------------------------------------------------
#
# This file is auto generated from cmake. Do not manually modify content!
#
# ----------------------------------------------------------------------------

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from wheel.bdist_wheel import bdist_wheel
from shutil import copyfile
from distutils.errors import *
import sys
import setuptools
import os
import distutils.ccompiler

__version__ = '${NGRAPH_VERSION_SHORT}'

# Parallel build from http://stackoverflow.com/questions/11013851/speeding-up-build-process-with-distutils
# monkey-patch for parallel compilation
def parallelCCompile(self, sources, output_dir=None, macros=None, include_dirs=None,
                     debug=0, extra_preargs=None, extra_postargs=None, depends=None):
    # those lines are copied from distutils.ccompiler.CCompiler directly
    macros, objects, extra_postargs, pp_opts, build = self._setup_compile(
        output_dir, macros, include_dirs, sources, depends, extra_postargs)
    cc_args = self._get_cc_args(pp_opts, debug, extra_preargs)
    # parallel code
    import multiprocessing.pool

    def _single_compile(obj):
        try:
            src, ext = build[obj]
        except KeyError:
            return
        self._compile(obj, src, ext, cc_args, extra_postargs, pp_opts)
    # convert to list, imap is evaluated on-demand
    list(multiprocessing.pool.ThreadPool().imap(_single_compile, objects))
    return objects


original_compile=distutils.ccompiler.CCompiler.compile

if sys.version_info < (3, 6):
    distutils.ccompiler.CCompiler.compile=parallelCCompile


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """
    Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    retval = True
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            original_compile(compiler, sources=[f.name], extra_postargs=[flagname])
        except CompileError:
            retval = False
    return retval


def cpp_flag(compiler):
    """Check and return the -std=c++11 compiler flag.

    """
    if has_flag(compiler, '-std=c++11'):
        return '-std=c++11'
    else:
        raise RuntimeError('Unsupported compiler -- C++11 support is needed!')


PYNGRAPH_SOURCE_DIR = '${CMAKE_CURRENT_SOURCE_DIR}'
PYBIND11_INCLUDE_DIR = '${PYBIND11_SOURCE_DIR}/include'
NGRAPH_CPP_INCLUDE_DIR = '${NGRAPH_DEPS_INCLUDE}'
NGRAPH_CPP_LIBRARY_DIR = '${NGRAPH_DEPS_LIB}'

pyngraph_prefix = PYNGRAPH_SOURCE_DIR + '/'
sources = []
packages = []
package_dir = dict()

for root, dirs, files in os.walk(pyngraph_prefix):
    if root.startswith(pyngraph_prefix + 'pyngraph'):
        sources += [root + '/' + f for f in files if f.endswith('.cpp')]
    elif root.startswith(pyngraph_prefix + 'ngraph'):
        if '${NGRAPH_ONNX_IMPORT_ENABLE}' not in ['TRUE', 'ON', True] and 'onnx_import' in root:
            continue
        if '__init__.py' in files:
            package_name = root[len(pyngraph_prefix):].replace('/', '.')
            packages += [package_name]
            package_dir[package_name] = root

include_dirs = [PYNGRAPH_SOURCE_DIR,
                NGRAPH_CPP_INCLUDE_DIR,
                PYBIND11_INCLUDE_DIR,
               ]

library_dirs = [NGRAPH_CPP_LIBRARY_DIR,
               ]

libraries    = ['ngraph',
               ]

extra_compile_args = []
if '${NGRAPH_ONNX_IMPORT_ENABLE}' in ['TRUE', 'ON', True]:
    extra_compile_args.append('-DNGRAPH_ONNX_IMPORT_ENABLE')

extra_link_args = []

data_files = [
    (
        'licenses',
        [
            '${CMAKE_SOURCE_DIR}/licenses/' + license
            for license in os.listdir('${CMAKE_SOURCE_DIR}/licenses')
        ],
    ),
    (
        '',
        ['${CMAKE_SOURCE_DIR}/LICENSE'],
    )
]

if sys.platform == 'win32':
    lib_suffix = '.dll'
elif sys.platform.startswith('linux'):
    lib_suffix = '.so'
elif sys.platform == 'darwin':
    lib_suffix = '.dylib'
else:
    raise RuntimeError('Unsupported platform!')

sharedlib_files = [NGRAPH_CPP_LIBRARY_DIR + '/' + library for library in os.listdir(NGRAPH_CPP_LIBRARY_DIR) if lib_suffix in library]

ext_modules = [Extension(
                   '_pyngraph',
                   sources = sources,
                   include_dirs = include_dirs,
                   define_macros = [('VERSION_INFO', __version__)],
                   library_dirs = library_dirs,
                   libraries = libraries,
                   extra_compile_args = extra_compile_args,
                   extra_link_args = extra_link_args,
                   language = 'c++',
                   )
              ]


class BuildExt(build_ext):
    """
    A custom build extension for adding compiler-specific options.
    """
    def build_extensions(self):
        if sys.platform == 'win32':
            raise RuntimeError('Unsupported platform: win32!')
        if not os.path.exists(self.build_lib + '/'):
            os.makedirs(self.build_lib)
        for source in sharedlib_files:
            destination = self.build_lib + '/' + os.path.basename(source)
            if 'libomp' in source or 'libgomp' in source:
                continue
            copyfile(source, destination)
            if 'libtbb' in destination:
                continue
            if sys.platform.startswith('linux'):
                rpath_patch_cmd = "patchelf --force-rpath --set-rpath '$ORIGIN' " + destination
            else:
                rpath_patch_cmd = "install_name_tool -id \"@rpath\" " + destination
            if os.system(rpath_patch_cmd) != 0:
                raise Exception("Failed to patch rpath of %s" % destination)
        """-Wstrict-prototypes is not a valid option for c++"""
        try:
            self.compiler.compiler_so.remove("-Wstrict-prototypes")
        except (AttributeError, ValueError):
            pass
        for ext in self.extensions:
            ext.extra_compile_args += [cpp_flag(self.compiler)]
            if has_flag(self.compiler, '-fstack-protector-strong'):
                ext.extra_compile_args += ['-fstack-protector-strong']
            elif has_flag(self.compiler, '-fstack-protector'):
                ext.extra_compile_args += ['-fstack-protector']
            if has_flag(self.compiler, '-fvisibility=hidden'):
                ext.extra_compile_args += ['-fvisibility=hidden']
            if has_flag(self.compiler, '-flto'):
                ext.extra_compile_args += ['-flto']
            if has_flag(self.compiler, '-fPIC'):
                ext.extra_compile_args += ['-fPIC']
            if sys.platform.startswith('linux'):
                ext.extra_link_args += ['-Wl,-rpath,$ORIGIN']
                ext.extra_link_args += ['-z', 'noexecstack']
                ext.extra_link_args += ['-z', 'relro']
                ext.extra_link_args += ['-z', 'now']
            elif sys.platform == 'darwin':
                ext.extra_link_args += ["-Wl,-rpath,@loader_path"]
            ext.extra_compile_args += ['-Wformat', '-Wformat-security']
            ext.extra_compile_args += ['-O2', '-D_FORTIFY_SOURCE=2']
        build_ext.build_extensions(self)


with open(PYNGRAPH_SOURCE_DIR + '/requirements.txt') as req:
    requirements = req.read().splitlines()


class BdistWheel(bdist_wheel):
    def get_tag(self):
        tag = bdist_wheel.get_tag(self)
        if '${NGRAPH_MANYLINUX_ENABLE}' == 'TRUE' and sys.platform.startswith('linux'):
            tag = tag[:2] + ('manylinux1_x86_64',) + tag[3:]
        return tag


setup(
    name='ngraph-core',
    description='nGraph - Intel\'s graph compiler and runtime for Neural Networks',
    version=__version__,
    author='Intel',
    author_email='intelnervana@intel.com',
    url='https://github.com/NervanaSystems/ngraph/',
    license='License :: OSI Approved :: Apache Software License',
    long_description=open(os.path.join(PYNGRAPH_SOURCE_DIR, 'README.md')).read(),
    long_description_content_type='text/markdown',
    ext_modules=ext_modules,
    package_dir=package_dir,
    packages=packages,
    cmdclass={'build_ext': BuildExt, 'bdist_wheel': BdistWheel},
    data_files=data_files,
    setup_requires=['numpy'],
    install_requires=requirements,
    zip_safe=False,
)