#include <unistd.h>
#include <strings.h>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <mutex>
#include <algorithm>
#include "file_lock.h"
#include "compile_command_recorder.h"

namespace ctrace {

CompileCommandRecorder::CompileCommandRecorder() :
        kSourceExtensions_({".s", ".c", ".cc", ".cpp", ".cxx", ".c++"}),
        kDrivers_({"cc", "c++", "gcc", "g++", "clang", "clang++"}),
        kExcludedDrivers_({"cc1plus", "cc1"})
        {}

bool CompileCommandRecorder::isCompileCommand(const char* executableFilePath) {
  // If ancestor is a compile command, skip current process.
  if (getenv(kEnvDoesParentFindCompileCommand_)) {
    return false;
  }

  std::string execPath = executableFilePath;

  // "/usr/bin/gcc", find the last '/'
  size_t compareStart = execPath.find_last_of(kPathDelimiter_);
  compareStart = (compareStart == std::string::npos) ? 0 : compareStart + 1;

  std::string name = execPath.substr(compareStart);
  for (const auto& driver : kDrivers_) {
    if (strncmp(name.c_str(), driver.c_str(), driver.size()) == 0
      && std::find(kExcludedDrivers_.begin(), kExcludedDrivers_.end(), name) == kExcludedDrivers_.end()) {
      return true;
    }
  }
  return false;
}

ssize_t CompileCommandRecorder::findFirstSource(char** paramList, size_t size) {
  for (size_t p = 0; p < size; p++) {
    std::string param = paramList[p];
    for (const auto& ext : kSourceExtensions_) {
      if (param.size() < ext.size()) {
        continue;
      }
      if (strncasecmp(
          ext.c_str(), param.c_str() + param.size() - ext.size(), ext.size()) == 0) {
        return p;
      }
    }
  }
  return -1;
}

bool CompileCommandRecorder::markCompileCommandFound() {
  return setenv(kEnvDoesParentFindCompileCommand_, "1", 0) == 0 ? true : false;
}

bool CompileCommandRecorder::recordCompileCommand(int argc, char** argv, char* source) {
    const char* file = getenv(kEnvCompileCommandsJson_);
    assert(file != nullptr && file[0] != '\0');

    FILE *stream = fopen(file, "a");
    assert(stream != nullptr);

    {
      FileLock lock(fileno(stream));
      std::lock_guard<FileLock> guard(lock);
      logCompileCommand(stream, argc, argv, source);
      fflush(stream);
    }
    fclose(stream);
}

bool CompileCommandRecorder::logCompileCommand(FILE* stream, int argc, char** argv, char* source) {
  fputs("  {\n", stream);  
  {
    char cwd[FILENAME_MAX];
     fputs("    \"directory\": \"", stream);
     putAll(stream, getcwd(cwd, FILENAME_MAX) ? cwd : "");
     fputs("\",\n", stream);
  }
  {
    char path[PATH_MAX];
    fputs("    \"file\": \"", stream);
    putAll(stream, realpath(source, path) ? path : "");
    fputs("\",\n", stream);
  }
  {
    fputs("    \"command\": \"", stream);
    putAll(stream, argv[0]);  
    for (int p = 1; p < argc; p++) {
      putc_unlocked(' ', stream);
      putAll(stream, argv[p]);  
    }
    fputs("\"\n", stream);
  }
  fputs("  },\n", stream);
}

void CompileCommandRecorder::putAll(FILE* stream, const char* data) {
  for (size_t c = 0; data[c] != '\0'; ++c) {
    CompileCommandRecorder::putc(stream, data[c]);
  }
}

void CompileCommandRecorder::putc(FILE* stream, char ch) {
  #define CASE(in, out)                  \
    case in:                            \
      putc_unlocked('\\', stream);      \
      putc_unlocked(out, stream);        \
      break;
   /* A JSON string can contain any Unicode character other than '"' or '\' or
    * a control character.  Encode control characters specially.  Assume the
    * bytes [0x01...0x1F, 0x7F] correspond to codepoints
    * [U+0001...U+001F, U+007F], which is correct for UTF-8. */
  switch(ch) {
     CASE('"',  '"')
     CASE('\\', '\\')
     CASE('\b', 'b')
     CASE('\f', 'f')
     CASE('\n', 'n')
     CASE('\r', 'r')
     CASE('\t', 't')
     default: {
        const unsigned char uch = ch;
        if (uch <= 31 || uch == 0x7f) {
            putc_unlocked('\\', stream);
            putc_unlocked('u', stream);
            putc_unlocked('0', stream);
            putc_unlocked('0', stream);
            putc_unlocked("0123456789abcdef"[uch >> 4], stream);
            putc_unlocked("0123456789abcdef"[uch & 0xf], stream);
        } else {
            putc_unlocked(ch, stream);
        }
    }
  }
}

}
