forth: builtins.o dos.o errors.o forth.o input_stream.o memory.o stack.o util.o
	gcc -g -o forth $^

builtins.o: builtins.c builtins.h
	gcc -g -c $<

dos.o: dos.c dos.h
	gcc -g -c $<

errors.o: errors.c errors.h
	gcc -g -c $<

forth.o: forth.c forth.h
	gcc -g -c $<

input_stream.o: input_stream.c input_stream.h
	gcc -g -c $<

memory.o: memory.c memory.h
	gcc -g -c $<

stack.o: stack.c stack.h
	gcc -g -c $<

util.o: util.c util.h
	gcc -g -c $<

clean:
	rm -f *.o forth