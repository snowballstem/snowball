# -*- makefile -*-

c_src_dir = src_c
java_src_main_dir = java/org/tartarus/snowball
java_src_dir = $(java_src_main_dir)/ext
python ?= python3
python_output_dir = python_out
python_runtime_dir = snowballstemmer
python_sample_dir = sample
jsx_output_dir = jsx_out
jsx_runtime_src_dir = jsx
jsx_runtime_dir = lib
jsx_sample_dir = sample
ICONV = iconv
#ICONV = python ./iconv.py

libstemmer_algorithms = danish dutch english finnish french german hungarian \
			italian \
			norwegian porter portuguese romanian \
			russian spanish swedish turkish

KOI8_R_algorithms = russian
ISO_8859_1_algorithms = danish dutch english finnish french german italian \
			norwegian porter portuguese spanish swedish
ISO_8859_2_algorithms = hungarian romanian

other_algorithms = german2 kraaij_pohlmann lovins

all_algorithms = $(libstemmer_algorithms) $(other_algorithms)

COMPILER_SOURCES = compiler/space.c \
		   compiler/tokeniser.c \
		   compiler/analyser.c \
		   compiler/generator.c \
		   compiler/driver.c \
		   compiler/generator_java.c \
		   compiler/generator_jsx.c \
		   compiler/generator_python.c

COMPILER_HEADERS = compiler/header.h \
		   compiler/syswords.h \
		   compiler/syswords2.h

RUNTIME_SOURCES  = runtime/api.c \
		   runtime/utilities.c

RUNTIME_HEADERS  = runtime/api.h \
		   runtime/header.h

JAVARUNTIME_SOURCES = java/org/tartarus/snowball/Among.java \
		      java/org/tartarus/snowball/SnowballProgram.java \
		      java/org/tartarus/snowball/SnowballStemmer.java \
		      java/org/tartarus/snowball/TestApp.java

JSX_RUNTIME_SOURCES = jsx/among.jsx \
		      jsx/base-stemmer.jsx \
		      jsx/stemmer.jsx

JSX_SAMPLE_SOURCES = jsx/testapp.jsx \
		     jsx/stemwords.jsx

PYTHON_RUNTIME_SOURCES = python/snowballstemmer/basestemmer.py \
		         python/snowballstemmer/among.py

PYTHON_SAMPLE_SOURCES = python/testapp.py \
		        python/stemwords.py

PYTHON_PACKAGE_FILES = python/MANIFEST.in \
		       python/setup.py

LIBSTEMMER_SOURCES = libstemmer/libstemmer.c
LIBSTEMMER_UTF8_SOURCES = libstemmer/libstemmer_utf8.c
LIBSTEMMER_HEADERS = include/libstemmer.h libstemmer/modules.h libstemmer/modules_utf8.h
LIBSTEMMER_EXTRA = libstemmer/modules.txt libstemmer/modules_utf8.txt libstemmer/libstemmer_c.in

STEMWORDS_SOURCES = examples/stemwords.c

JSX_STEMWORDS_SOURCE = jsx/stemwords.jsx

PYTHON_STEMWORDS_SOURCE = python/stemwords.py

ALL_ALGORITHM_FILES = $(all_algorithms:%=algorithms/%/stem*.sbl)
C_LIB_SOURCES = $(libstemmer_algorithms:%=$(c_src_dir)/stem_UTF_8_%.c) \
		$(KOI8_R_algorithms:%=$(c_src_dir)/stem_KOI8_R_%.c) \
		$(ISO_8859_1_algorithms:%=$(c_src_dir)/stem_ISO_8859_1_%.c) \
		$(ISO_8859_2_algorithms:%=$(c_src_dir)/stem_ISO_8859_2_%.c)
C_LIB_HEADERS = $(libstemmer_algorithms:%=$(c_src_dir)/stem_UTF_8_%.h) \
		$(KOI8_R_algorithms:%=$(c_src_dir)/stem_KOI8_R_%.h) \
		$(ISO_8859_1_algorithms:%=$(c_src_dir)/stem_ISO_8859_1_%.h) \
		$(ISO_8859_2_algorithms:%=$(c_src_dir)/stem_ISO_8859_2_%.h)
C_OTHER_SOURCES = $(other_algorithms:%=$(c_src_dir)/stem_UTF_8_%.c)
C_OTHER_HEADERS = $(other_algorithms:%=$(c_src_dir)/stem_UTF_8_%.h)
JAVA_SOURCES = $(libstemmer_algorithms:%=$(java_src_dir)/%Stemmer.java)
PYTHON_SOURCES = $(libstemmer_algorithms:%=$(python_output_dir)/%_stemmer.py) \
		 $(python_output_dir)/__init__.py
JSX_SOURCES = $(libstemmer_algorithms:%=$(jsx_output_dir)/%-stemmer.jsx)

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)
RUNTIME_OBJECTS=$(RUNTIME_SOURCES:.c=.o)
LIBSTEMMER_OBJECTS=$(LIBSTEMMER_SOURCES:.c=.o)
LIBSTEMMER_UTF8_OBJECTS=$(LIBSTEMMER_UTF8_SOURCES:.c=.o)
STEMWORDS_OBJECTS=$(STEMWORDS_SOURCES:.c=.o)
C_LIB_OBJECTS = $(C_LIB_SOURCES:.c=.o)
C_OTHER_OBJECTS = $(C_OTHER_SOURCES:.c=.o)
JAVA_CLASSES = $(JAVA_SOURCES:.java=.class)
JAVA_RUNTIME_CLASSES=$(JAVARUNTIME_SOURCES:.java=.class)

CFLAGS=-O2 -W -Wall -Wmissing-prototypes -Wmissing-declarations
CPPFLAGS=-Iinclude

all: snowball libstemmer.o stemwords $(C_OTHER_SOURCES) $(C_OTHER_HEADERS) $(C_OTHER_OBJECTS)

clean:
	rm -f $(COMPILER_OBJECTS) $(RUNTIME_OBJECTS) \
	      $(LIBSTEMMER_OBJECTS) $(LIBSTEMMER_UTF8_OBJECTS) $(STEMWORDS_OBJECTS) snowball \
	      libstemmer.o stemwords \
              libstemmer/modules.h \
              libstemmer/modules_utf8.h \
              snowball.splint \
	      $(C_LIB_SOURCES) $(C_LIB_HEADERS) $(C_LIB_OBJECTS) \
	      $(C_OTHER_SOURCES) $(C_OTHER_HEADERS) $(C_OTHER_OBJECTS) \
	      $(JAVA_SOURCES) $(JAVA_CLASSES) $(JAVA_RUNTIME_CLASSES) \
	      $(PYTHON_SOURCES) \
	      $(JSX_SOURCES) jsx_stemwords \
              libstemmer/mkinc.mak libstemmer/mkinc_utf8.mak \
              libstemmer/libstemmer.c libstemmer/libstemmer_utf8.c
	rm -rf dist
	rmdir $(c_src_dir) || true
	rmdir $(python_output_dir) || true
	rmdir $(jsx_output_dir) || true

snowball: $(COMPILER_OBJECTS)
	$(CC) -o $@ $^

$(COMPILER_OBJECTS): $(COMPILER_HEADERS)

libstemmer/libstemmer.c: libstemmer/libstemmer_c.in
	sed 's/@MODULES_H@/modules.h/' $^ >$@

libstemmer/libstemmer_utf8.c: libstemmer/libstemmer_c.in
	sed 's/@MODULES_H@/modules_utf8.h/' $^ >$@

libstemmer/modules.h libstemmer/mkinc.mak: libstemmer/mkmodules.pl libstemmer/modules.txt
	libstemmer/mkmodules.pl $@ $(c_src_dir) libstemmer/modules.txt libstemmer/mkinc.mak

libstemmer/modules_utf8.h libstemmer/mkinc_utf8.mak: libstemmer/mkmodules.pl libstemmer/modules_utf8.txt
	libstemmer/mkmodules.pl $@ $(c_src_dir) libstemmer/modules_utf8.txt libstemmer/mkinc_utf8.mak utf8

libstemmer/libstemmer.o: libstemmer/modules.h $(C_LIB_HEADERS)

libstemmer.o: libstemmer/libstemmer.o $(RUNTIME_OBJECTS) $(C_LIB_OBJECTS)
	$(AR) -cru $@ $^

stemwords: $(STEMWORDS_OBJECTS) libstemmer.o
	$(CC) -o $@ $^

jsx_stemwords: $(JSX_STEMWORDS_SOURCE) $(JSX_SOURCES)
	jsx --executable node --output $@ --add-search-path $(jsx_output_dir) --add-search-path $(jsx_runtime_src_dir) $(JSX_STEMWORDS_SOURCE)

algorithms/%/stem_Unicode.sbl: algorithms/%/stem_ISO_8859_1.sbl
	cp $^ $@

$(c_src_dir)/stem_UTF_8_%.c $(c_src_dir)/stem_UTF_8_%.h: algorithms/%/stem_Unicode.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_Unicode.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_UTF_8_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_UTF_8_ -r ../runtime -u"; \
	./snowball $< -o $${o} -eprefix $${l}_UTF_8_ -r ../runtime -u

$(c_src_dir)/stem_KOI8_R_%.c $(c_src_dir)/stem_KOI8_R_%.h: algorithms/%/stem_KOI8_R.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_KOI8_R.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_KOI8_R_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_KOI8_R_ -r ../runtime"; \
	./snowball $< -o $${o} -eprefix $${l}_KOI8_R_ -r ../runtime

$(c_src_dir)/stem_ISO_8859_1_%.c $(c_src_dir)/stem_ISO_8859_1_%.h: algorithms/%/stem_ISO_8859_1.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_ISO_8859_1.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_ISO_8859_1_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_ISO_8859_1_ -r ../runtime"; \
	./snowball $< -o $${o} -eprefix $${l}_ISO_8859_1_ -r ../runtime

$(c_src_dir)/stem_ISO_8859_2_%.c $(c_src_dir)/stem_ISO_8859_2_%.h: algorithms/%/stem_ISO_8859_2.sbl snowball
	@mkdir -p $(c_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_ISO_8859_2.sbl$$!\1!;s!^.*/!!'`; \
	o="$(c_src_dir)/stem_ISO_8859_2_$${l}"; \
	echo "./snowball $< -o $${o} -eprefix $${l}_ISO_8859_2_ -r ../runtime"; \
	./snowball $< -o $${o} -eprefix $${l}_ISO_8859_2_ -r ../runtime

$(c_src_dir)/stem_%.o: $(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(java_src_dir)/%Stemmer.java: algorithms/%/stem_Unicode.sbl snowball
	@mkdir -p $(java_src_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_Unicode.sbl$$!\1!;s!^.*/!!'`; \
	o="$(java_src_dir)/$${l}Stemmer"; \
	echo "./snowball $< -j -o $${o} -p \"org.tartarus.snowball.SnowballStemmer\" -eprefix $${l}_ -r ../runtime -n $${l}Stemmer"; \
	./snowball $< -j -o $${o} -p "org.tartarus.snowball.SnowballStemmer" -eprefix $${l}_ -r ../runtime -n $${l}Stemmer

$(python_output_dir)/%_stemmer.py: algorithms/%/stem_Unicode.sbl snowball
	@mkdir -p $(python_output_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_Unicode.sbl$$!\1!;s!^.*/!!'`; \
	o="$(python_output_dir)/$${l}_stemmer"; \
	echo "./snowball $< -py -o $${o} -p \"SnowballStemmer\" -eprefix $${l}_ -r ../runtime -n `$(python) -c "print('$${l}'.title())"`Stemmer"; \
	./snowball $< -py -o $${o} -p "BaseStemmer" -eprefix $${l}_ -r ../runtime -n `$(python) -c "print('$${l}'.title())"`Stemmer

$(python_output_dir)/__init__.py:
	@mkdir -p $(python_output_dir)
	$(python) python/create_init.py $(python_output_dir)

$(jsx_output_dir)/%-stemmer.jsx: algorithms/%/stem_Unicode.sbl snowball
	@mkdir -p $(jsx_output_dir)
	@l=`echo "$<" | sed 's!\(.*\)/stem_Unicode.sbl$$!\1!;s!^.*/!!'`; \
	o="$(jsx_output_dir)/$${l}-stemmer"; \
	echo "./snowball $< -jsx -o $${o} -p \"SnowballStemmer\" -eprefix $${l}_ -r ../runtime -n `$(python) -c "print('$${l}'.title())"`Stemmer"; \
	./snowball $< -jsx -o $${o} -p "BaseStemmer" -eprefix $${l}_ -r ../runtime -n `$(python) -c "print('$${l}'.title())"`Stemmer

splint: snowball.splint
snowball.splint: $(COMPILER_SOURCES)
	splint $^ >$@ -weak

# Make a full source distribution
dist: dist_snowball dist_libstemmer_c dist_libstemmer_java dist_libstemmer_jsx dist_libstemmer_python

# Make a distribution of all the sources involved in snowball
dist_snowball: $(COMPILER_SOURCES) $(COMPILER_HEADERS) \
	    $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
	    $(LIBSTEMMER_SOURCES) \
	    $(LIBSTEMMER_UTF8_SOURCES) \
            $(LIBSTEMMER_HEADERS) \
	    $(LIBSTEMMER_EXTRA) \
	    $(ALL_ALGORITHM_FILES) $(STEMWORDS_SOURCES) \
	    GNUmakefile README doc/TODO libstemmer/mkmodules.pl
	destname=snowball_code; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	for file in $^; do \
	  dir=`dirname $$file` && \
	  mkdir -p $${dest}/$${dir} && \
	  cp -a $${file} $${dest}/$${dir} || exit 1 ; \
	done && \
	(cd dist && tar zcf $${destname}.tgz $${destname}) && \
	rm -rf $${dest}

# Make a distribution of all the sources required to compile the C library.
dist_libstemmer_c: \
            $(RUNTIME_SOURCES) \
            $(RUNTIME_HEADERS) \
            $(LIBSTEMMER_SOURCES) \
            $(LIBSTEMMER_UTF8_SOURCES) \
            $(LIBSTEMMER_HEADERS) \
            $(LIBSTEMMER_EXTRA) \
	    $(C_LIB_SOURCES) \
            $(C_LIB_HEADERS) \
            libstemmer/mkinc.mak \
            libstemmer/mkinc_utf8.mak
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
	cp -a $(LIBSTEMMER_SOURCES) $(LIBSTEMMER_UTF8_SOURCES) $(LIBSTEMMER_HEADERS) $(LIBSTEMMER_EXTRA) $${dest}/libstemmer && \
	mkdir -p $${dest}/include && \
	mv $${dest}/libstemmer/libstemmer.h $${dest}/include && \
	(cd $${dest} && \
	 echo "README" >> MANIFEST && \
	 ls $(c_src_dir)/*.c $(c_src_dir)/*.h >> MANIFEST && \
	 ls runtime/*.c runtime/*.h >> MANIFEST && \
	 ls libstemmer/*.c libstemmer/*.h >> MANIFEST && \
	 ls include/*.h >> MANIFEST) && \
        cp -a libstemmer/mkinc.mak libstemmer/mkinc_utf8.mak $${dest}/ && \
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
            $(LIBSTEMMER_EXTRA) \
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

dist_libstemmer_python: $(PYTHON_SOURCES)
	destname=snowballstemmer; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	echo "a1" && \
	mkdir -p $${dest} && \
	mkdir -p $${dest}/src/$(python_runtime_dir) && \
	mkdir -p $${dest}/src/$(python_sample_dir) && \
	cp doc/libstemmer_python_README $${dest}/README.rst && \
	cp -a $(PYTHON_SOURCES) $${dest}/src/$(python_runtime_dir) && \
	cp -a $(PYTHON_SAMPLE_SOURCES) $${dest}/src/$(python_sample_dir) && \
	cp -a $(PYTHON_RUNTIME_SOURCES) $${dest}/src/$(python_runtime_dir) && \
	cp -a $(PYTHON_PACKAGE_FILES) $${dest} && \
	(cd $${dest} && $(python) setup.py sdist && cp dist/*.tar.gz ..) && \
	rm -rf $${dest}

dist_libstemmer_jsx: $(JSX_SOURCES)
	destname=jsxstemmer; \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}.tgz && \
	mkdir -p $${dest} && \
	mkdir -p $${dest}/$(jsx_runtime_dir) && \
	mkdir -p $${dest}/$(jsx_sample_dir) && \
	cp -a doc/libstemmer_jsx_README $${dest}/README && \
	cp -a $(JSX_RUNTIME_SOURCES) $${dest}/$(jsx_runtime_dir) && \
	cp -a $(JSX_SAMPLE_SOURCES) $${dest}/$(jsx_sample_dir) && \
	cp -a $(JSX_SOURCES) $${dest}/$(jsx_runtime_dir) && \
	(cd $${dest} && \
	 echo "README" >> MANIFEST && \
	 ls $(jsx_runtime_dir)/*.jsx >> MANIFEST && \
	 ls $(jsx_sample_dir)/*.jsx >> MANIFEST) && \
	(cd dist && tar zcf $${destname}.tgz $${destname}) && \
	rm -rf $${dest}

check: check_utf8 check_iso_8859_1 check_iso_8859_2 check_koi8r

check_utf8: $(libstemmer_algorithms:%=check_utf8_%)

check_iso_8859_1: $(ISO_8859_1_algorithms:%=check_iso_8859_1_%)

check_iso_8859_2: $(ISO_8859_2_algorithms:%=check_iso_8859_2_%)

check_koi8r: $(KOI8_R_algorithms:%=check_koi8r_%)

# Where the data files are located - assumed their repo is checked out as
# a sibling to this one.
STEMMING_DATA = ../snowball-data

check_utf8_%: $(STEMMING_DATA)/% stemwords
	@echo "Checking output of `echo $<|sed 's!.*/!!'` stemmer with UTF-8"
	@./stemwords -c UTF_8 -l `echo $<|sed 's!.*/!!'` -i $</voc.txt -o tmp.txt
	@diff -u $</output.txt tmp.txt
	@rm tmp.txt

check_iso_8859_1_%: $(STEMMING_DATA)/% stemwords
	@echo "Checking output of `echo $<|sed 's!.*/!!'` stemmer with ISO_8859_1"
	@$(ICONV) -fUTF8 -tISO8859-1 '$</voc.txt' |\
	    ./stemwords -c ISO_8859_1 -l `echo $<|sed 's!.*/!!'` -o tmp.txt
	@$(ICONV) -fUTF8 -tISO8859-1 '$</output.txt' |\
	    diff -u - tmp.txt
	@rm tmp.txt

check_iso_8859_2_%: $(STEMMING_DATA)/% stemwords
	@echo "Checking output of `echo $<|sed 's!.*/!!'` stemmer with ISO_8859_2"
	@$(ICONV) -fUTF8 -tISO8859-2 '$</voc.txt' |\
	    ./stemwords -c ISO_8859_2 -l `echo $<|sed 's!.*/!!'` -o tmp.txt
	@$(ICONV) -fUTF8 -tISO8859-2 '$</output.txt' |\
	    diff -u - tmp.txt
	@rm tmp.txt

check_koi8r_%: $(STEMMING_DATA)/% stemwords
	@echo "Checking output of `echo $<|sed 's!.*/!!'` stemmer with KOI8R"
	@$(ICONV) -fUTF8 -tKOI8-R '$</voc.txt' |\
	    ./stemwords -c KOI8_R -l `echo $<|sed 's!.*/!!'` -o tmp.txt
	@$(ICONV) -fUTF8 -tKOI8-R '$</output.txt' |\
	    diff -u - tmp.txt
	@rm tmp.txt

check_jsx: $(libstemmer_algorithms:%=check_jsx_%)

check_jsx_%: $(STEMMING_DATA)/% jsx_stemwords
	@echo "Checking output of `echo $<|sed 's!.*/!!'` stemmer with UTF-8"
	@./jsx_stemwords -c utf8 -l `echo $<|sed 's!.*/!!'` -i $</voc.txt -o tmp.txt
	@diff -u $</output.txt tmp.txt
	@rm tmp.txt

check_python: check_python_stemwords $(libstemmer_algorithms:%=check_python_%)

check_python_%: $(STEMMING_DATA)/%
	@echo "Checking output of `echo $<|sed 's!.*/!!'` stemmer with UTF-8"
	(cd python_check && \
	$(python) stemwords.py -c utf8 -l `echo $<|sed 's!.*/!!'` -i ../$</voc.txt -o tmp.txt && \
	diff -u ../$</output.txt tmp.txt && \
	rm tmp.txt)

check_python_stemwords: $(PYTHON_STEMWORDS_SOURCE) $(PYTHON_SOURCES)
	mkdir -p python_check && \
	mkdir -p python_check/snowballstemmer && \
	cp -a $(PYTHON_RUNTIME_SOURCES) python_check/snowballstemmer && \
	cp -a $(PYTHON_SOURCES) python_check/snowballstemmer && \
	cp -a $(PYTHON_STEMWORDS_SOURCE) python_check/
