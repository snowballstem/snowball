# -*- makefile -*-

c_src_dir = src_c
java_src_main_dir = java/org/tartarus/snowball
java_src_dir = $(java_src_main_dir)/ext

libstemmer_algorithms = danish dutch english finnish french german hungarian \
			italian \
			norwegian porter portuguese russian spanish swedish

KOI8_R_algorithms = russian
ISO_8859_1_algorithms = danish dutch english finnish french german hungarian \
			italian \
			norwegian porter portuguese spanish swedish

other_algorithms = german2 kraaij_pohlmann romanian1 romanian2 lovins

all_algorithms = $(libstemmer_algorithsm) $(other_algorithms)

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

JAVARUNTIME_SOURCES = java/org/tartarus/snowball/Among.java \
		      java/org/tartarus/snowball/SnowballProgram.java \
		      java/org/tartarus/snowball/TestApp.java

LIBSTEMMER_SOURCES = libstemmer/libstemmer.c
LIBSTEMMER_HEADERS = include/libstemmer.h libstemmer/modules.h

STEMWORDS_SOURCES = examples/stemwords.c

ALGORITHMS = $(all_algorithms:%=algorithms/%/stem.sbl)
C_LIB_SOURCES = $(libstemmer_algorithms:%=$(c_src_dir)/stem_UTF_8_%.c) \
		$(KOI8_R_algorithms:%=$(c_src_dir)/stem_KOI8_R_%.c) \
		$(ISO_8859_1_algorithms:%=$(c_src_dir)/stem_ISO_8859_1_%.c)
C_LIB_HEADERS = $(libstemmer_algorithms:%=$(c_src_dir)/stem_UTF_8_%.h) \
		$(KOI8_R_algorithms:%=$(c_src_dir)/stem_KOI8_R_%.h) \
		$(ISO_8859_1_algorithms:%=$(c_src_dir)/stem_ISO_8859_1_%.h)
C_OTHER_SOURCES = $(other_algorithms:%=$(c_src_dir)/stem_UTF_8_%.c)
C_OTHER_HEADERS = $(other_algorithms:%=$(c_src_dir)/stem_UTF_8_%.h)
JAVA_SOURCES = $(libstemmer_algorithms:%=$(java_src_dir)/%Stemmer.java)

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)
RUNTIME_OBJECTS=$(RUNTIME_SOURCES:.c=.o)
LIBSTEMMER_OBJECTS=$(LIBSTEMMER_SOURCES:.c=.o)
STEMWORDS_OBJECTS=$(STEMWORDS_SOURCES:.c=.o)
C_LIB_OBJECTS = $(C_LIB_SOURCES:.c=.o)
C_OTHER_OBJECTS = $(C_OTHER_SOURCES:.c=.o)
JAVA_CLASSES = $(JAVA_SOURCES:.java=.class)
JAVA_RUNTIME_CLASSES=$(JAVARUNTIME_SOURCES:.java=.class)

CFLAGS=-Iinclude
CPPFLAGS=-W -Wall -Wmissing-prototypes -Wmissing-declarations

all: snowball libstemmer.o stemwords $(C_OTHER_OBJECTS)

clean:
	rm -f $(COMPILER_OBJECTS) $(RUNTIME_OBJECTS) \
	      $(LIBSTEMMER_OBJECTS) $(STEMWORDS_OBJECTS) snowball \
	      libstemmer.o stemwords libstemmer/modules.h snowball.splint \
	      $(C_LIB_SOURCES) $(C_LIB_HEADERS) $(C_LIB_OBJECTS) \
	      $(C_OTHER_SOURCES) $(C_OTHER_HEADERS) $(C_OTHER_OBJECTS) \
	      $(JAVA_SOURCES) $(JAVA_CLASSES) $(JAVA_RUNTIME_CLASSES)
	rm -rf dist
	rmdir $(c_src_dir) || true

snowball: $(COMPILER_OBJECTS)
	$(CC) -o $@ $^

libstemmer/modules.h: libstemmer/mkmodules.pl libstemmer/modules.txt
	libstemmer/mkmodules.pl $@ $(c_src_dir) libstemmer/modules.txt

libstemmer/libstemmer.o: libstemmer/modules.h $(C_LIB_HEADERS)

libstemmer.o: libstemmer/libstemmer.o $(RUNTIME_OBJECTS) $(C_LIB_OBJECTS)
	$(AR) -cru $@ $^

stemwords: $(STEMWORDS_OBJECTS) libstemmer.o
	$(CC) -o $@ $^

algorithms/%/stem-Unicode.sbl: algorithms/%/stem.sbl
	cp $^ $@
algorithms/russian/stem-KOI8-R.sbl: algorithms/russian/stem.sbl
	cp $^ $@
algorithms/%/stem-ISO-8859-1.sbl: algorithms/%/stem.sbl
	cp $^ $@

$(c_src_dir)/stem_UTF_8_%.c $(c_src_dir)/stem_UTF_8_%.h: algorithms/%/stem-Unicode.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem-Unicode.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_UTF_8_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_UTF_8_ -r ../runtime -u"; \
	./snowball $< -o $${o} -eprefix $${l}_UTF_8_ -r ../runtime -u

$(c_src_dir)/stem_KOI8_R_%.c $(c_src_dir)/stem_KOI8_R_%.h: algorithms/%/stem-KOI8-R.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem-KOI8-R.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_KOI8_R_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_KOI8_R_ -r ../runtime"; \
	./snowball $< -o $${o} -eprefix $${l}_KOI8_R_ -r ../runtime

$(c_src_dir)/stem_ISO_8859_1_%.c $(c_src_dir)/stem_ISO_8859_1_%.h: algorithms/%/stem-ISO-8859-1.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem-ISO-8859-1.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_ISO_8859_1_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_ISO_8859_1_ -r ../runtime"; \
	./snowball $< -o $${o} -eprefix $${l}_ISO_8859_1_ -r ../runtime

$(c_src_dir)/stem_%.o: $(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h
	$(CC) $(CFLAGS) -O4 -c -o $@ $< -Wall

$(java_src_dir)/%Stemmer.java: algorithms/%/stem-Unicode.sbl snowball
	@mkdir -p $(java_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem-Unicode.sbl$$!\1!;s!^.*/!!'`; \
	o="$(java_src_dir)/$${l}Stemmer"; \
	echo "./snowball $< -j -o $${o} -eprefix $${l}_ -r ../runtime -n $${l}Stemmer"; \
	./snowball $< -j -o $${o} -eprefix $${l}_ -r ../runtime -n $${l}Stemmer

splint: snowball.splint
snowball.splint: $(COMPILER_SOURCES)
	splint $^ >$@ -weak

# Make a full source distribution
dist: dist_snowball dist_libstemmer_c dist_libstemmer_java

# Make a distribution of all the sources involved in snowball
dist_snowball: $(COMPILER_SOURCES) $(COMPILER_HEADERS) \
	    $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
	    $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_HEADERS) \
	    $(ALGORITHMS) $(STEMWORDS_SOURCES) \
	    GNUmakefile README doc/TODO libstemmer/mkmodules.pl
	destname=snowball_code; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	for file in $^; do \
	  dir=`dirname $$file` && \
	  mkdir -p $${dest}/$${dir} && \
	  cp $${file} $${dest}/$${dir} || exit 1 ; \
	done && \
	(cd dist && tar zcf $${destname}.tgz $${destname}) && \
	rm -rf $${dest}

# Make a distribution of all the sources required to compile the C library.
dist_libstemmer_c: $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
            $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_HEADERS) \
	    $(C_LIB_SOURCES) $(C_LIB_HEADERS)
	destname=libstemmer_c; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	mkdir -p $${dest} && \
	cp -a doc/libstemmer_c_README $${dest}/README && \
	mkdir -p $${dest}/examples && \
	cp -a examples/stemwords.c $${dest}/examples && \
	mkdir -p $${dest}/$(c_src_dir) && \
	cp -a $(C_LIB_SOURCES) $(C_LIB_HEADERS) $${dest}/$(c_src_dir) && \
	mkdir -p $${dest}/runtime && \
	cp -a $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) $${dest}/runtime && \
	mkdir -p $${dest}/libstemmer && \
	cp -a $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_HEADERS) $${dest}/libstemmer && \
	mkdir -p $${dest}/include && \
	mv $${dest}/libstemmer/libstemmer.h $${dest}/include && \
	(cd $${dest} && \
	 echo "README" >> MANIFEST && \
	 ls $(c_src_dir)/*.c $(c_src_dir)/*.h >> MANIFEST && \
	 ls runtime/*.c runtime/*.h >> MANIFEST && \
	 ls libstemmer/*.c libstemmer/*.h >> MANIFEST && \
	 ls include/*.h >> MANIFEST && \
	 echo 'snowball_sources= \' >> mkinc.mak && \
	 ls $(c_src_dir)/*.c runtime/*.c libstemmer/*.c \
	  | perl -ne 'print " \\\n" if $$neednl;chomp($$_);print $$_;$$neednl=1' >> mkinc.mak && \
	 echo >> mkinc.mak && \
	 echo >> mkinc.mak && \
	 echo 'snowball_headers= \' >> mkinc.mak && \
	 ls $(c_src_dir)/*.h runtime/*.h libstemmer/*.h include/*.h \
	  | perl -ne 'print " \\\n" if $$neednl;chomp($$_);print $$_;$$neednl=1' >> mkinc.mak && \
	 echo >> mkinc.mak && \
	 echo >> mkinc.mak) && \
	echo 'include mkinc.mak' >> $${dest}/Makefile && \
	echo 'CFLAGS=-Iinclude' >> $${dest}/Makefile && \
	echo 'all: libstemmer.o stemwords' >> $${dest}/Makefile && \
	echo 'libstemmer.o: $$(snowball_sources:.c=.o)' >> $${dest}/Makefile && \
	echo '	$$(AR) -cru $$@ $$^' >> $${dest}/Makefile && \
	echo 'stemwords: examples/stemwords.o libstemmer.o' >> $${dest}/Makefile && \
	echo '	$$(CC) -o $$@ $$^' >> $${dest}/Makefile && \
	echo 'clean:' >> $${dest}/Makefile && \
	echo '	rm -f stemwords *.o $(c_src_dir)/*.o runtime/*.o libstemmer/*.o' >> $${dest}/Makefile && \
	(cd dist && tar zcf $${destname}.tgz $${destname}) && \
	rm -rf $${dest}

# Make a distribution of all the sources required to compile the Java library.
dist_libstemmer_java: $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
            $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_HEADERS) \
	    $(JAVA_SOURCES)
	destname=libstemmer_java; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	mkdir -p $${dest} && \
	cp -a doc/libstemmer_java_README $${dest}/README && \
	mkdir -p $${dest}/$(java_src_dir) && \
	cp -a $(JAVA_SOURCES) $${dest}/$(java_src_dir) && \
	mkdir -p $${dest}/$(java_src_main_dir) && \
	cp -a $(JAVARUNTIME_SOURCES) $${dest}/$(java_src_main_dir) && \
	(cd $${dest} && \
	 echo "README" >> MANIFEST && \
	 ls $(java_src_dir)/*.java >> MANIFEST && \
	 ls $(java_src_main_dir)/*.java >> MANIFEST) && \
	(cd dist && tar zcf $${destname}.tgz $${destname}) && \
	rm -rf $${dest}
