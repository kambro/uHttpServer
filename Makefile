# build executable when user executes "make"
ALL: uhttpserver

LD=-L/usr/lib  -lssl -lcrypto -lpthread
X_GCC_COPTIONS=-Wbad-function-cast -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -Wtraditional -Wstrict-prototypes  -Wtraditional
Y_GCC_COPTIONS=-Wbad-function-cast -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -Wstrict-prototypes 

GCC_COPTIONS=-Wbad-function-cast -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -Wstrict-prototypes -ftime-report 

GCC_ADDIONAL_OPTIONS=-Ddebug=1

duhttpserver: uhttpserver

uhttpserver: uhttpserver.o common.o server.o str_tools.o config.o
	$(CC) $(LDFLAGS) uhttpserver.o common.o server.o config.o string_tools.o -o uhttpserver $(LD)
uhttpserver.o: 
	$(CC) $(CFLAGS) $(GCC_COPTIONS) -c -g uhttpserver.c
common.o: 
	$(CC) $(CFLAGS) $(GCC_COPTIONS) -c -g common.c
server.o: 
	$(CC) $(CFLAGS) $(GCC_COPTIONS) -c -g server.c
config.o: 
	$(CC) $(CFLAGS) $(GCC_COPTIONS) -c -g config.c
str_tools.o:
	$(CC) $(CFLAGS) $(GCC_COPTIONS) -c -g string_tools.c
clean:
	rm  *.o uhttpserver
