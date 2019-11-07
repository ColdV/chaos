cc = g++ -std=c++11 -g
prom = NetFrame
deps = $(shell find ./ -name "*.h")
src = $(shell find ./ -name "*.cpp" -o -name "*.c")
obj = $(src: %.c = %.o)
lib = -lpthread
$(prom): $(obj)
	$(cc) -o $(prom) $(obj) $(lib)

%.o: %.c %.cpp $(deps)
	&(cc) -c &< -o &@

clean:
	rm -rf $(obj) $(prom)
