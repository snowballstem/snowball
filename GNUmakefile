# -*- makefile -*-

c_src_dir = src_c

languages = danish dutch english finnish french german italian lovins \
            norwegian porter portuguese russian spanish swedish
lang_aliases = da=danish \
	       de=german \
	       en=english \
	       english_lovins=lovins \
	       english_porter=porter \
	       es=spanish \
	       fi=finnish \
	       fr=french \
	       it=italian \
 	       nl=dutch \
	       no=norwegian \
	       pt=portuguese \
	       ru=russian \
	       sv=swedish

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

LIBSTEMMER_SOURCES = libstemmer/libstemmer.c \
		     libstemmer/modules.h

STEMWORDS_SOURCES = examples/stemwords.c

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)
RUNTIME_OBJECTS=$(RUNTIME_SOURCES:.c=.o)
LIBSTEMMER_OBJECTS=$(LIBSTEMMER_SOURCES:.c=.o)
STEMWORDS_OBJECTS=$(STEMWORDS_SOURCES:.c=.o)

CFLAGS=-Ilibstemmer

all: snowball libstemmer.o stemwords

clean:
	rm -f $(COMPILER_OBJECTS) $(RUNTIME_OBJECTS) \
	      $(LIBSTEMMER_OBJECTS) $(STEMWORDS_OBJECTS) snowball \
	      libstemmer.o stemwords libstemmer/modules.h snowball.splint \
	      $(languages:%=$(c_src_dir)/stem_%.h) \
	      $(languages:%=$(c_src_dir)/stem_%.c) \
	      $(languages:%=$(c_src_dir)/stem_%.o)
	rmdir $(c_src_dir)

snowball: $(COMPILER_OBJECTS)
	$(CC) -o $@ $^

libstemmer/modules.h: libstemmer/mkmodules.pl
	libstemmer/mkmodules.pl $@ $(c_src_dir) $(languages) $(lang_aliases)

libstemmer/libstemmer.o: libstemmer/modules.h $(languages:%=$(c_src_dir)/stem_%.h)

libstemmer.o: libstemmer/libstemmer.o $(RUNTIME_OBJECTS) $(languages:%=$(c_src_dir)/stem_%.o)
	$(AR) -cru $@ $^

stemwords: $(STEMWORDS_OBJECTS) libstemmer.o
	$(CC) -o $@ $^

$(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h: algorithms/%/stem.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_"; \
	./snowball $< -o $${o} -eprefix $${l}_

$(c_src_dir)/stem_%.o: $(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h
	$(CC) $(CFLAGS) -O4 -c -o $@ -I runtime/ $<

splint: snowball.splint
snowball.splint: $(COMPILER_SOURCES)
	splint $^ >$@ -weak
