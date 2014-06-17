OPTIONS=-Wno-unused-result -Werror=declaration-after-statement -DNDEBUG -g -fwrapv -fwrapv -O3 -Wall -Wstrict-prototypes
INCLUDES=-I/sw/lib/python3.4/site-packages/numpy/core/include/numpy -I/sw/include/python3.4m
LIBS = -L/sw/lib/python3.4/config-3.4m -L/sw/lib -lpython3.4m 

.PHONY: clean veryclean

odbparser.so: odbparsermodule.o odb_io.o odb_io_f.o 
	$(CC) -bundle $(LIBS) $^ -o $@

odb_io.o: odb_io.c odb_io.h
	$(CC) $(OPTIONS) -c $< -o $@

odb_io_f.o: odb_io_f.c  odb_io.h
	$(CC) $(OPTIONS) -c $< -o $@

odbparsermodule.o: odbparsermodule.c odb_io.h
	$(CC) $(OPTIONS) $(INCLUDES) -c $< -o $@

clean:
	rm -f odb_io.o odb_io_f.o odbparsermodule.o

veryclean: clean
	rm -f odbparser.so *~
