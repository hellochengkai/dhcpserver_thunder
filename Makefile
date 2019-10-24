CC     = /opt/toolchains/android-toolchain_r14b/bin/arm-linux-androideabi-gcc
#CC     = gcc
CFLAGS = -pie -fPIE -Wall -ggdb
OBJS   = args.o bindings.o dhcpserver.o options.o

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

dhcpserver: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(OBJS) dhcpserver
