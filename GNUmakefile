# -*- makefile -*-

languages=english porter

COMPILER_SOURCES = compiler/space.c \
                   compiler/sort.c \
		   compiler/tokeniser.c \
		   compiler/analyser.c \
		   compiler/generator.c \
		   compiler/driver.c \
		   compiler/generator_java.c
COMPILER_HEADERS = compiler/header.h \
		   compiler/syswords \
		   compiler/syswords2

RUNTIME_SOURCES  = runtime/api.c \
		   runtime/utilities.c
RUNTIME_HEADERS  = runtime/api.h \
		   runtime/header.h

MKMODULES_SOURCES = libstemmer/mkmodules.c

LIBSTEMMER_SOURCES = libstemmer/libstemmer.c \
		     libstemmer/modules.h

STEMWORDS_SOURCES = examples/stemwords.c

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)
RUNTIME_OBJECTS=$(RUNTIME_SOURCES:.c=.o)
MKMODULES_OBJECTS=$(MKMODULES_SOURCES:.c=.o)
LIBSTEMMER_OBJECTS=$(LIBSTEMMER_SOURCES:.c=.o)
STEMWORDS_OBJECTS=$(STEMWORDS_SOURCES:.c=.o)

CFLAGS=-Ilibstemmer

all: snowball libstemmer.o stemwords

clean:
	rm -f $(COMPILER_OBJECTS) $(RUNTIME_OBJECTS) $(MKMODULES_OBJECTS) \
	      $(LIBSTEMMER_OBJECTS) $(STEMWORDS_OBJECTS) snowball mkmodules \
	      libstemmer.o stemwords libstemmer/modules.h snowball.splint

snowball: $(COMPILER_OBJECTS)
	$(CC) -o $@ $^

mkmodules: $(MKMODULES_SOURCES)
	$(CC) -o $@ $^

libstemmer/modules.h: mkmodules
	./mkmodules $@ $(languages)

libstemmer/libstemmer.o: libstemmer/modules.h

libstemmer.o: libstemmer/libstemmer.o $(RUNTIME_OBJECTS) $(languages:=/stem.o)
	$(AR) -cru $@ $^

stemwords: $(STEMWORDS_OBJECTS) libstemmer.o
	$(CC) -o $@ $^

%/stem.c: %/stem.sbl snowball
	@l=`echo "$<" | sed 's!\(.*\)/stem.sbl$$!\1!;s!^.*/!!'`; \
	./snowball $< -o $${l}/stem -eprefix $${l}_

%/stem.o: %/stem.c %/stem.h
	$(CC) $(CFLAGS) -O4 -c -o $@ -I runtime/ $<

splint: snowball.splint
snowball.splint: $(COMPILER_SOURCES)
	splint $^ >$@ -weak
