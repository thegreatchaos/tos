bin/tos:tos.cc
	mkdir -p bin
	g++ -std=c++11 $^ -o $@
clean:
	-rm -rf bin/
run:bin/tos
	./bin/tos
