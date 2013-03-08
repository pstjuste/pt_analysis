CPPFLAGS = -Ilemon/include -I.
LDFLAGS = -Llemon/lib -lemon
PROGS = ijsn13 dupedges csr13 sigcomm2012 subgraph socialgraph degree

all : $(PROGS)

% : %.cpp
	g++ -g -static-libstdc++ -o $@ $< $(CPPFLAGS) $(LDFLAGS)

clean :
	rm $(PROGS)
