#include <climits>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <mutex>
#include <cstdio>
#include "compile_command_recorder.h"

/**
 * To use tools offered by clang to do code analysis, it is usually required to offer a
 * compile command database file named compile_commands.json. For pro that use cmake,
 * it's easy since cmake supports to export compile_commands.json by compiling with
 * -DCMAKE_EXPORT_COMPILE_COMMANDS=1 macro. However, for projects that don't use cmake,
 * it's not that easy to generate compile_commands.json. So here we write this tool, hoping
 * it might help.
 *
 * To generate the compile_commands.json, we need first get each process' command line
 * arguments during the while compile process, then filter out the non-compiling commands.
 * There are several ways to get one process' command line arguments:
 * 1. Use a fake cc, cxx to replace the environment variable CC and CXX, and record the
 *    command line arguments in the fake cc/cxx. This works most of the time, but fails
 *    when the target project use an absolute path to specify C and C++ compile instead
 *    of using CC and CXX.
 * 2. For linux, use LD_PRELOAD to preload a dynamic library, this library does nothing
 *    except reading command line arguments from /proc/self/cmdline. This is not portable,
 *    but on other platforms, we have similar solution.
 *    This solution works 99% of the time except when the command line arguments are too
 *    long (longer than 4K). Linux allows only the first 4K of the command line arguments
 *    to be logged in /proc/self/cmdline, if the command line arguments are longer than
 *    this size, it is truncated.
 *
 *
 */
void libEntry(int argc, char** argv, char** envp) {
  ctrace::CompileCommandRecorder recorder;
  if (!recorder.isCompileCommand(argv[0])) {
    return; 
  }
  ssize_t pos = recorder.findFirstSource(argv, argc);
  if (pos < 0) {
    return;
  }
  recorder.markCompileCommandFound();
  recorder.recordCompileCommand(argc, argv, argv[pos]);
}

__attribute__((section(".init_array"))) decltype(libEntry) *__init = libEntry;
