CFLAGS=-Wall -c -g -fPIC -D__LINUX__
INCLUDE=-I../libs/libevent-2.0.10-stable/include/
LDFLAGS=-shared -Wl,-soname,libteteco.so.1 -lportaudio -lspeex -lspeexdsp -levent
LIBS=-L/home/nacho/teteco/lib/
CC=gcc

SOURCES=src/teteco_net.c \
        src/audio.c \
        src/enc_speex.c \
        src/frame_list.c \
        src/util.c \
        src/speex_jitter_buffer.c \
        src/protocol.c \
        src/chat.c \
        src/log.c \
        src/teteco.c

OBJECTS=$(SOURCES:.c=.o)
LIBRARY=src/libteteco.so.1.2


all: $(SOURCES) $(LIBRARY)
	ln -sf $(LIBRARY) src/libteteco.so.1
	ln -sf $(LIBRARY) src/libteteco.so

$(LIBRARY): $(OBJECTS)
	$(CC) $(LIBS) $(LDFLAGS) $(OBJECTS)  -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@


clean:
	rm -f src/*.so* src/*.o


deb:
	./debianize.sh


.PHONY: debian
