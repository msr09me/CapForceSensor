default: init build
build: init ${LIBOUT}
rebuild: clean init build

${LIBOUT}:   ${OBJS}
	${AR} crs ${LIBOUT} $(OBJS)
	mv ${LIBOUT} ${OUTPUTS}/lib/

.c.o: ${HDRS}
	${CC} ${CFLAGS} -c $*.c

.cpp.o: ${HDRS}
	${CPP} ${CPPFLAGS} -c $*.cpp

init:
	cp ${HDRS} ${OUTPUTS}/include/
clean:
	rm -f $(OBJS) 
	