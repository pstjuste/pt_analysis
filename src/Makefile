CPPFLAGS = -Ilemon/include -I.
LDFLAGS = -Llemon/lib -lemon
PROGS = sigcomm2012 subgraph socialgraph degree

all : $(PROGS)

% : %.cpp
	g++ -g -o $@ $< $(CPPFLAGS) $(LDFLAGS)

clean :
	rm $(PROGS)
