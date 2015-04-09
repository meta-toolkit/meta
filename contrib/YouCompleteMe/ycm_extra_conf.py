# .ycm_extra_conf.py for MeTA source code.
# Based on the provided ycm_extra_conf from YouCompleteMe as well as the
# ycm_extra_conf.py from neovim.
import os
import ycm_core

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )

def GetDatabase():
  build_folder = os.path.join(DirectoryOfThisScript(), 'build')
  if (os.path.exists(build_folder)):
    return ycm_core.CompilationDatabase(build_folder)
  return None

def MakeRelativePathsInFlagsAbsolute(flags, working_directory):
  if not working_directory:
    return flags
  new_flags = []
  make_next_absolute = False
  path_flags = ['-isystem', '-I', '-iquote', '--sysroot=']
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith('/'):
        new_flag = os.path.join(working_directory, flag)

    for path_flag in path_flags:
      if flag == path_flag:
        make_next_absolute = True
        break

      if flag.startswith(path_flag):
        path = flag[len(path_flag):]
        new_flag = path_flag + os.path.join(working_directory, path)
        break

    if new_flag:
      new_flags.append(new_flag)
  return new_flags

def IsHeader(filename):
  headers = set(['.h', '.hpp', '.tcc'])
  return os.path.splitext(filename)[1] in headers

def GetCompilationInfoForFile(filename):
  database = GetDatabase()
  if not database:
    return None

  if IsHeader(filename):
    # CMake doesn't generate flags for header files in its compilation
    # database, so we attempt to look up the corresponding cpp file in the
    # source directory and use its flags
    base_name = os.path.splitext(filename)[0]
    cpp_name = base_name.replace('/include', '/src') + '.cpp'

    # if we can't find the corresponding cpp, fall back to the flags for
    # profile.cpp
    if not os.path.exists(cpp_name):
      cpp_name = os.path.join(DirectoryOfThisScript(), 'src', 'tools',
          'profile.cpp')

    if not os.path.exists(cpp_name):
      return None

    compilation_info = database.GetCompilationInfoForFile(cpp_name)
    # fixes issues where .h files seemingly can't find c++ standard header
    # files
    if compilation_info.compiler_flags_:
      compilation_info.compiler_flags_.append('-x')
      compilation_info.compiler_flags_.append('c++')
    return compilation_info

  compilation_info = database.GetCompilationInfoForFile(filename)
  # if we can't find this file in our database, fall back to the flags for
  # profile.cpp
  if not compilation_info.compiler_flags_:
    cpp_name = os.path.join(DirectoryOfThisScript(), 'src', 'tools',
          'profile.cpp')
    compilation_info = database.GetCompilationInfoForFile(cpp_name)

  return compilation_info

def FlagsForFile( filename ):
  compilation_info = GetCompilationInfoForFile(filename)
  if not compilation_info:
    return None

  final_flags = MakeRelativePathsInFlagsAbsolute(
      compilation_info.compiler_flags_,
      compilation_info.compiler_working_dir_)

  return {
    'flags': final_flags,
    'do_cache': True
  }
