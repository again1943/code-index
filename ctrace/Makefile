CXX=g++

ctrace: ctrace.cpp compile_command_recorder.cpp
	${CXX} ctrace.cpp compile_command_recorder.cpp -o libctrace.so -shared -fPIC -std=c++11

clean:
	rm -rf *.so
