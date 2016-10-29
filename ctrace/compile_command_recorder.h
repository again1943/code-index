#include <cstdio>
#include <string>
#include <vector>

namespace ctrace {

class CompileCommandRecorder{
public:
  CompileCommandRecorder();
public:
  // Determine the executable file path is a c/c++ compile command.
  bool isCompileCommand(const char* executableFilePath);

  // Find the first source file(ending with ".cpp", ".cc", ".cxx", ".c", ..., etc)
  // in the param list, return the index other wise -1
  ssize_t findFirstSource(char** paramList, size_t size);

  // If we have found that current process is a compile command, any child process
  // of current process will be ignored. This function sets the ignore flag by adding
  // an environment variable for child processes.
  bool markCompileCommandFound();

  // Record the compile command in json format.
  bool recordCompileCommand(int argc, char** argv, char* source);

private:
  bool logCompileCommand(FILE* stream, int argc, char** argv, char* source);

  void putAll(FILE* stream, const char* data);

  void putc(FILE* stream, char ch);
private:
  static constexpr const char* kEnvDoesParentFindCompileCommand_ = "FOUND_COMPILE_COMMAND";
	static constexpr const char* kEnvCompileCommandsJson_ = "COMPILE_COMMANDS_JSON";
  static constexpr const char kPathDelimiter_ = '/';
private:
  const std::vector<std::string> kSourceExtensions_;
  const std::vector<std::string> kDrivers_;
  const std::vector<std::string> kExcludedDrivers_;
};

}
