CFLAGS=--std=c99 -g

forth: builtins.o dos.o errors.o forth.o input_stream.o memory.o stack.o util.o
	gcc $(CFLAGS) -o forth $^

builtins.o: builtins.c builtins.h
	gcc $(CFLAGS) -c $<

dos.o: dos.c dos.h
	gcc $(CFLAGS) -c $<

errors.o: errors.c errors.h
	gcc $(CFLAGS) -c $<

forth.o: forth.c forth.h
	gcc $(CFLAGS) -c $<

input_stream.o: input_stream.c input_stream.h
	gcc $(CFLAGS) -c $<

memory.o: memory.c memory.h
	gcc $(CFLAGS) -c $<

stack.o: stack.c stack.h
	gcc $(CFLAGS) -c $<

util.o: util.c util.h
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o forth