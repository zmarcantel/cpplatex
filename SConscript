import os
import re
import fnmatch
import multiprocessing

LIBPATH = [
    # insert libs here
    os.path.join(GetOption('outdir'), 'lib'),
]
if GetOption('travis'):
    LIBPATH.extend(['/home/travis/gtest'])

LIBS = [
    # add dependency names here as they should be linked against
]

COMMONFLAGS=[
    # add language/compiler agnostic flags here+
    '-fPIC',
    '-Wabi',
    '-msse4.2',
    '-Wno-unused-parameter',
    '-Wall',
    '-Wextra',
    '-Werror',
]

CPPPATH = [
    # add c++ paths here
]

CPPFLAGS = [
    # required C++ flags
    '-std=c++14' if not GetOption('travis') and not GetOption('clang') else '-std=c++1y',
]

GCC_CPPFLAGS = [
    # TODO: GCC 4.9 give ABI incompatibility warnings upgraded to errors  without this
    '-fabi-version=10',
]

CXXCOMPILER='g++' if not GetOption('travis') else 'g++-5'
CCCOMPILER='gcc' if not GetOption('travis') else 'gcc-5'

def append_flags(src, to):
    for i in src:
        if i not in to:
            to.append(i)

# what will actually be used to build
FLAGSET = COMMONFLAGS

if GetOption('clang'):
    CXXCOMPILER='clang++' if not GetOption('travis') else '/usr/local/clang-3.4/bin/clang++'
    CCCOMPILER='clang' if not GetOption('travis') else '/usr/local/clang-3.4/bin/clang'
else:
    append_flags(GCC_CPPFLAGS, FLAGSET)


##
## Helpers
##

def get_files(env, root, exts):
    #directory of SConstruct
    rootpath = Dir('#').abspath
    srcroot = os.path.join(rootpath, root)
    if not os.path.exists(srcroot):
        raise Exception('{} does not exist'.format(srcroot))

    src = []
    filt = re.compile('.*\.({})$'.format('|'.join(exts)))
    for root, dirs, files in os.walk(srcroot, followlinks=True):
        for filename in files:
            if filt.match(filename):
                src.append(os.path.join(root, filename)[len(srcroot)+1:])
    return src
AddMethod(Environment, get_files)


##
## Build Targets
##

builds = [
    'test',
]
for b in builds:
    idir =  os.path.join(Dir('#').abspath, b)
    ldir = os.path.join(Dir('#').abspath, GetOption('outdir'), os.path.basename(b))

    CPPFLAGS.extend(['-I{}'.format(idir)])
    LIBPATH.extend([ldir])


##
## Build Entry
##
SetOption('num_jobs', int(multiprocessing.cpu_count()))

CPPFLAGS.extend(COMMONFLAGS),
global_env = Environment(
    LIBS=LIBS,
    LIBPATH=LIBPATH,
    CPPPATH=CPPPATH,
    CC=CCCOMPILER,
    CXX=CXXCOMPILER,
    CXXFLAGS=CPPFLAGS,

    CCCOMSTR       = "[ CC  ]\t$TARGET" if not GetOption('ugly') else None,
    CXXCOMSTR      = "[ CPP ]\t$TARGET" if not GetOption('ugly') else None,
    LINKCOMSTR     = "[ LD  ]\t$TARGET" if not GetOption('ugly') else None,
    ARCOMSTR       = "[ AR  ]\t$TARGET" if not GetOption('ugly') else None,
    RANLIBCOMSTR   = "[ LIB ]\t$TARGET" if not GetOption('ugly') else None,
)

Export('global_env')

build_targets = {}
for build in builds:
    out = '/'.join(build.split('/')[1:])
    build_targets[build] = SConscript(os.path.join(build, "SConscript"), variant_dir=out, duplicate=False)

