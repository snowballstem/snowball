# -*- makefile -*-

c_src_dir = src_c

languages = danish dutch english finnish french german german2 italian lovins \
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

LIBSTEMMER_SOURCES = libstemmer/libstemmer.c
LIBSTEMMER_HEADERS = include/libstemmer.h libstemmer/modules.h

STEMWORDS_SOURCES = examples/stemwords.c

C_SOURCES = $(languages:%=$(c_src_dir)/stem_%.c)
C_HEADERS = $(languages:%=$(c_src_dir)/stem_%.h)

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)
RUNTIME_OBJECTS=$(RUNTIME_SOURCES:.c=.o)
LIBSTEMMER_OBJECTS=$(LIBSTEMMER_SOURCES:.c=.o)
STEMWORDS_OBJECTS=$(STEMWORDS_SOURCES:.c=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)

CFLAGS=-Iinclude
CPPFLAGS=-W -Wall -Wmissing-prototypes -Wmissing-declarations -Werror

all: snowball libstemmer.o stemwords

clean:
	rm -f $(COMPILER_OBJECTS) $(RUNTIME_OBJECTS) \
	      $(LIBSTEMMER_OBJECTS) $(STEMWORDS_OBJECTS) snowball \
	      libstemmer.o stemwords libstemmer/modules.h snowball.splint \
	      $(C_SOURCES) $(C_HEADERS) $(C_OBJECTS)
	rm -rf dist
	rmdir $(c_src_dir)

snowball: $(COMPILER_OBJECTS)
	$(CC) -o $@ $^

libstemmer/modules.h: libstemmer/mkmodules.pl
	libstemmer/mkmodules.pl $@ $(c_src_dir) $(languages) $(lang_aliases)

libstemmer/libstemmer.o: libstemmer/modules.h $(C_HEADERS)

libstemmer.o: libstemmer/libstemmer.o $(RUNTIME_OBJECTS) $(C_OBJECTS)
	$(AR) -cru $@ $^

stemwords: $(STEMWORDS_OBJECTS) libstemmer.o
	$(CC) -o $@ $^

$(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h: algorithms/%/stem.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_ -r ../runtime"; \
	./snowball $< -o $${o} -eprefix $${l}_ -r ../runtime

$(c_src_dir)/stem_%.o: $(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h
	$(CC) $(CFLAGS) -O4 -c -o $@ $<

splint: snowball.splint
snowball.splint: $(COMPILER_SOURCES)
	splint $^ >$@ -weak

# Make a full source distribution
dist:
	@echo "UNIMPLEMENTED"

# Make a distribution of all the sources required to compile the C library.
c-src-dist: $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
            $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_HEADERS) \
	    $(C_SOURCES) $(C_HEADERS)
	destname=snowball_c_src; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	mkdir -p $${dest}/$(c_src_dir) && \
	cp -a $(C_SOURCES) $(C_HEADERS) $${dest}/$(c_src_dir) && \
	mkdir -p $${dest}/runtime && \
	cp -a $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) $${dest}/runtime && \
	mkdir -p $${dest}/libstemmer && \
	cp -a $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_HEADERS) $${dest}/libstemmer && \
	mkdir -p $${dest}/include && \
	mv $${dest}/libstemmer/libstemmer.h $${dest}/include && \
	(cd $${dest} && \
	 ls $(c_src_dir)/*.c $(c_src_dir)/*.h >> MANIFEST && \
	 ls runtime/*.c runtime/*.h >> MANIFEST && \
	 ls libstemmer/*.c libstemmer/*.h >> MANIFEST && \
	 ls include/*.h >> MANIFEST && \
	 echo 'snowball_sources= \' >> mkinc.mak && \
	 ls $(c_src_dir)/*.c runtime/*.c libstemmer/*.c \
	  | sed 's/$$/ \\/' >> mkinc.mak) && \
	echo 'include mkinc.mak' >> $${dest}/Makefile && \
	echo 'libstemmer.o: $$(snowball_sources:.c=.o)' >> $${dest}/Makefile && \
	echo '	$$(AR) -cru $$@ $$^' >> $${dest}/Makefile && \
	echo 'clean:' >> $${dest}/Makefile && \
	echo '	rm -f *.o $(c_src_dir)/*.o runtime/*.o libstemmer/*.o' >> $${dest}/Makefile && \
	(cd dist && tar zcf $${destname}.tgz $${destname}) && \
	rm -rf $${dest}

