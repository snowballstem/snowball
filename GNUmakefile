# -*- makefile -*-

# After changing this, run `make update_version` to update various sources
# which hard-code it.
SNOWBALL_VERSION = 3.0.0

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif

c_src_dir = src_c

JAVACFLAGS ?=
JAVAC ?= javac
JAVA ?= java -ea
java_src_main_dir = java/org/tartarus/snowball
java_src_dir = $(java_src_main_dir)/ext

MONO ?= mono
MCS ?= mcs
csharp_src_main_dir = csharp/Snowball
csharp_src_dir = $(csharp_src_main_dir)/Algorithms
csharp_sample_dir = csharp/Stemwords

FPC ?= fpc
# Enable warnings, info, notes; select "FILE:LINE:" diagnostic format.
FPC_FLAGS ?= -veiwnr
pascal_src_dir = pascal

python ?= python3
python_output_dir = python_out
python_runtime_dir = snowballstemmer
python_sample_dir = sample

js_output_dir = js_out
js_runtime_dir = javascript
js_sample_dir = sample
JSRUN ?= node
JSTYPE ?= global

cargo ?= cargo
cargoflags ?= --release
rust_src_main_dir = rust/src
rust_src_dir = $(rust_src_main_dir)/snowball/algorithms

go ?= go
goflags ?= stemwords/algorithms.go stemwords/main.go
gofmt ?= gofmt
go_src_main_dir = go
go_src_dir = $(go_src_main_dir)/algorithms

gprbuild ?= gprbuild
ada_src_main_dir = ada
ada_src_dir = $(ada_src_main_dir)/algorithms

DIFF = diff
ifeq ($(OS),Windows_NT)
DIFF = diff --strip-trailing-cr
endif
ICONV = iconv
#ICONV = python ./iconv.py

# Where the data files are located - assumes their repo is checked out as
# a sibling to this one.
STEMMING_DATA ?= ../snowball-data
STEMMING_DATA_ABS := $(abspath $(STEMMING_DATA))

# Keep one in $(THIN_FACTOR) entries from gzipped vocabularies.
THIN_FACTOR ?= 3

ifneq (1,$(THIN_FACTOR))
ifneq (,$(THIN_FACTOR))
# Command to thin out the testdata.  Used for Python tests, which otherwise
# take a long time (unless you use pypy).
THIN_TEST_DATA := |awk '(FNR % $(THIN_FACTOR) == 0){print}'
endif
endif

tarball_ext = .tar.gz

# algorithms.mk is generated from libstemmer/modules.txt and defines:
# * libstemmer_algorithms
# * ISO_8859_1_algorithms
# * ISO_8859_2_algorithms
# * KOI8_R_algorithms
include algorithms.mk

other_algorithms = lovins

all_algorithms = $(libstemmer_algorithms) $(other_algorithms)

COMPILER_SOURCES = compiler/space.c \
		   compiler/tokeniser.c \
		   compiler/analyser.c \
		   compiler/generator.c \
		   compiler/driver.c \
		   compiler/generator_csharp.c \
		   compiler/generator_java.c \
		   compiler/generator_js.c \
		   compiler/generator_pascal.c \
		   compiler/generator_python.c \
		   compiler/generator_rust.c \
		   compiler/generator_go.c \
		   compiler/generator_ada.c

COMPILER_HEADERS = compiler/header.h \
		   compiler/syswords.h

RUNTIME_SOURCES  = runtime/api.c \
		   runtime/utilities.c

RUNTIME_HEADERS  = runtime/api.h \
		   runtime/header.h

JAVARUNTIME_SOURCES = java/org/tartarus/snowball/Among.java \
		      java/org/tartarus/snowball/SnowballProgram.java \
		      java/org/tartarus/snowball/SnowballStemmer.java \
		      java/org/tartarus/snowball/TestApp.java

CSHARP_RUNTIME_SOURCES = csharp/Snowball/Among.cs \
			 csharp/Snowball/Stemmer.cs \
			 csharp/Snowball/AssemblyInfo.cs

CSHARP_STEMWORDS_SOURCES = csharp/Stemwords/Program.cs

JS_RUNTIME_SOURCES = javascript/base-stemmer.js

JS_SAMPLE_SOURCES = javascript/stemwords.js

PASCAL_RUNTIME_SOURCES = pascal/SnowballProgram.pas

PASCAL_STEMWORDS_SOURCES = pascal/stemwords.dpr

PYTHON_RUNTIME_SOURCES = python/snowballstemmer/basestemmer.py \
		         python/snowballstemmer/among.py

PYTHON_SAMPLE_SOURCES = python/testapp.py \
		        python/stemwords.py

PYTHON_PACKAGE_FILES = python/MANIFEST.in \
		       python/pyproject.toml \
		       python/setup.py \
		       python/setup.cfg

LIBSTEMMER_SOURCES = libstemmer/libstemmer.c
LIBSTEMMER_UTF8_SOURCES = libstemmer/libstemmer_utf8.c
LIBSTEMMER_HEADERS = include/libstemmer.h libstemmer/modules.h libstemmer/modules_utf8.h
LIBSTEMMER_EXTRA = libstemmer/modules.txt libstemmer/libstemmer_c.in

STEMWORDS_SOURCES = examples/stemwords.c
STEMTEST_SOURCES = tests/stemtest.c

PYTHON_STEMWORDS_SOURCE = python/stemwords.py

COMMON_FILES = COPYING \
	       NEWS

ALL_ALGORITHM_FILES = $(all_algorithms:%=algorithms/%.sbl)
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
CSHARP_SOURCES = $(libstemmer_algorithms:%=$(csharp_src_dir)/%Stemmer.generated.cs)
PASCAL_SOURCES = $(ISO_8859_1_algorithms:%=$(pascal_src_dir)/%Stemmer.pas)
PYTHON_SOURCES = $(libstemmer_algorithms:%=$(python_output_dir)/%_stemmer.py) \
		 $(python_output_dir)/__init__.py
JS_SOURCES = $(libstemmer_algorithms:%=$(js_output_dir)/%-stemmer.js) \
	$(js_output_dir)/base-stemmer.js \
	$(libstemmer_algorithms:%=$(js_output_dir)/%-stemmer.esm.js) \
	$(js_output_dir)/base-stemmer.esm.js
RUST_SOURCES = $(libstemmer_algorithms:%=$(rust_src_dir)/%_stemmer.rs)
GO_SOURCES = $(libstemmer_algorithms:%=$(go_src_dir)/%_stemmer.go) \
	$(go_src_main_dir)/stemwords/algorithms.go
ADA_SOURCES = $(libstemmer_algorithms:%=$(ada_src_dir)/stemmer-%.ads) \
        $(libstemmer_algorithms:%=$(ada_src_dir)/stemmer-%.adb) \
        $(ada_src_dir)/stemmer-factory.ads $(ada_src_dir)/stemmer-factory.adb

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)
RUNTIME_OBJECTS=$(RUNTIME_SOURCES:.c=.o)
LIBSTEMMER_OBJECTS=$(LIBSTEMMER_SOURCES:.c=.o)
LIBSTEMMER_UTF8_OBJECTS=$(LIBSTEMMER_UTF8_SOURCES:.c=.o)
STEMWORDS_OBJECTS=$(STEMWORDS_SOURCES:.c=.o)
STEMTEST_OBJECTS=$(STEMTEST_SOURCES:.c=.o)
C_LIB_OBJECTS = $(C_LIB_SOURCES:.c=.o)
C_OTHER_OBJECTS = $(C_OTHER_SOURCES:.c=.o)
JAVA_CLASSES = $(JAVA_SOURCES:.java=.class)
JAVA_RUNTIME_CLASSES=$(JAVARUNTIME_SOURCES:.java=.class)

CFLAGS=-g -O2 -W -Wall -Wmissing-prototypes -Wmissing-declarations -Wshadow $(WERROR)
CPPFLAGS=

INCLUDES=-Iinclude

all: snowball$(EXEEXT) libstemmer.a stemwords$(EXEEXT) $(C_OTHER_SOURCES) $(C_OTHER_HEADERS) $(C_OTHER_OBJECTS)

algorithms.mk: libstemmer/mkalgorithms.pl libstemmer/modules.txt
	libstemmer/mkalgorithms.pl algorithms.mk libstemmer/modules.txt

clean:
	rm -f $(COMPILER_OBJECTS) $(RUNTIME_OBJECTS) \
	      $(LIBSTEMMER_OBJECTS) $(LIBSTEMMER_UTF8_OBJECTS) $(STEMWORDS_OBJECTS) snowball$(EXEEXT) \
	      libstemmer.a stemwords$(EXEEXT) \
              libstemmer/modules.h \
              libstemmer/modules_utf8.h \
	      $(C_LIB_SOURCES) $(C_LIB_HEADERS) $(C_LIB_OBJECTS) \
	      $(C_OTHER_SOURCES) $(C_OTHER_HEADERS) $(C_OTHER_OBJECTS) \
	      $(JAVA_SOURCES) $(JAVA_CLASSES) $(JAVA_RUNTIME_CLASSES) \
	      $(CSHARP_SOURCES) \
	      $(PASCAL_SOURCES) pascal/stemwords.dpr pascal/stemwords pascal/*.o pascal/*.ppu \
	      $(PYTHON_SOURCES) \
	      $(JS_SOURCES) \
	      $(RUST_SOURCES) \
	      $(ADA_SOURCES) ada/bin/generate ada/bin/stemwords \
	      stemtest$(EXEEXT) $(STEMTEST_OBJECTS) \
              libstemmer/mkinc.mak libstemmer/mkinc_utf8.mak \
              libstemmer/libstemmer.c libstemmer/libstemmer_utf8.c \
	      algorithms.mk
	rm -rf ada/obj dist
	-rmdir $(c_src_dir)
	-rmdir $(python_output_dir)
	-rmdir $(js_output_dir)

update_version:
	perl -pi -e 's/(SNOWBALL_VERSION.*?)\d+\.\d+\.\d+/$${1}$(SNOWBALL_VERSION)/' \
		compiler/header.h \
		csharp/Snowball/AssemblyInfo.cs \
		python/setup.py

everything: all java csharp pascal js rust go python ada

baseline-create: everything
	rm -rf *.baseline
	for d in src_c java csharp pascal js_out go python_out ada ; do cp -a $$d $$d.baseline ; done
	rm -rf *.baseline/*.o ada.baseline/obj pascal.baseline/*.ppu
	find java.baseline -name '*.class' -delete

baseline-diff:
	@for d in src_c java csharp pascal js_out go python_out ada ; do diff -ru -x'*.o' -x'obj' -x'*.ppu' -x'*.class' $$d.baseline $$d ; done

.PHONY: all clean update_version everything baseline-create baseline-diff

$(STEMMING_DATA)/% $(STEMMING_DATA_ABS)/%:
	@[ -f '$@' ] || { echo '$@: Test data not found'; echo 'Checkout the snowball-data repo as "$(STEMMING_DATA_ABS)"'; exit 1; }

snowball$(EXEEXT): $(COMPILER_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(COMPILER_OBJECTS): $(COMPILER_HEADERS)

libstemmer/libstemmer.c: libstemmer/libstemmer_c.in
	sed 's/@MODULES_H@/modules.h/' $^ >$@

libstemmer/libstemmer_utf8.c: libstemmer/libstemmer_c.in
	sed 's/@MODULES_H@/modules_utf8.h/' $^ >$@

libstemmer/modules.h libstemmer/mkinc.mak: libstemmer/mkmodules.pl libstemmer/modules.txt
	libstemmer/mkmodules.pl $@ $(c_src_dir) libstemmer/modules.txt libstemmer/mkinc.mak

libstemmer/modules_utf8.h libstemmer/mkinc_utf8.mak: libstemmer/mkmodules.pl libstemmer/modules.txt
	libstemmer/mkmodules.pl $@ $(c_src_dir) libstemmer/modules.txt libstemmer/mkinc_utf8.mak utf8

libstemmer/libstemmer.o: libstemmer/modules.h $(C_LIB_HEADERS)

libstemmer.a: libstemmer/libstemmer.o $(RUNTIME_OBJECTS) $(C_LIB_OBJECTS)
	$(AR) -cru $@ $^

examples/%.o: examples/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CPPFLAGS) -c -o $@ $<

stemwords$(EXEEXT): $(STEMWORDS_OBJECTS) libstemmer.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CPPFLAGS) -c -o $@ $<

stemtest$(EXEEXT): $(STEMTEST_OBJECTS) libstemmer.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

csharp_stemwords$(EXEEXT): $(CSHARP_STEMWORDS_SOURCES) $(CSHARP_RUNTIME_SOURCES) $(CSHARP_SOURCES)
	$(MCS) -unsafe -target:exe -out:$@ $(CSHARP_STEMWORDS_SOURCES) $(CSHARP_RUNTIME_SOURCES) $(CSHARP_SOURCES)

pascal/stemwords.dpr: pascal/stemwords-template.dpr libstemmer/modules.txt
	pascal/generate.pl $(ISO_8859_1_algorithms) < pascal/stemwords-template.dpr > $@

pascal/stemwords: $(PASCAL_STEMWORDS_SOURCES) $(PASCAL_RUNTIME_SOURCES) $(PASCAL_SOURCES)
	$(FPC) $(FPC_FLAGS) -o$@ -Mdelphi $(PASCAL_STEMWORDS_SOURCES)

$(c_src_dir)/stem_UTF_8_%.c $(c_src_dir)/stem_UTF_8_%.h: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(c_src_dir)
	./snowball $< -o "$(c_src_dir)/stem_UTF_8_$*" -eprefix $*_UTF_8_ -r ../runtime -u

$(c_src_dir)/stem_KOI8_R_%.c $(c_src_dir)/stem_KOI8_R_%.h: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(c_src_dir)
	./snowball charsets/KOI8-R.sbl $< -o "$(c_src_dir)/stem_KOI8_R_$*" -eprefix $*_KOI8_R_ -r ../runtime

$(c_src_dir)/stem_ISO_8859_1_%.c $(c_src_dir)/stem_ISO_8859_1_%.h: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(c_src_dir)
	./snowball $< -o "$(c_src_dir)/stem_ISO_8859_1_$*" -eprefix $*_ISO_8859_1_ -r ../runtime

$(c_src_dir)/stem_ISO_8859_2_%.c $(c_src_dir)/stem_ISO_8859_2_%.h: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(c_src_dir)
	./snowball charsets/ISO-8859-2.sbl $< -o "$(c_src_dir)/stem_ISO_8859_2_$*" -eprefix $*_ISO_8859_2_ -r ../runtime

$(c_src_dir)/stem_%.o: $(c_src_dir)/stem_%.c $(c_src_dir)/stem_%.h
	$(CC) $(CFLAGS) $(INCLUDES) $(CPPFLAGS) -c -o $@ $<

$(java_src_dir)/%Stemmer.java: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(java_src_dir)
	./snowball $< -j -o "$(java_src_dir)/$*Stemmer" -p org.tartarus.snowball.SnowballStemmer

$(csharp_src_dir)/%Stemmer.generated.cs: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(csharp_src_dir)
	./snowball $< -cs -o "$(csharp_src_dir)/$*Stemmer.generated"

$(pascal_src_dir)/%Stemmer.pas: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(pascal_src_dir)
	./snowball $< -pascal -o "$(pascal_src_dir)/$*Stemmer"

$(python_output_dir)/%_stemmer.py: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(python_output_dir)
	./snowball $< -py -o "$(python_output_dir)/$*_stemmer"

$(python_output_dir)/__init__.py: python/create_init.py $(libstemmer_algorithms:%=$(python_output_dir)/%_stemmer.py)
	$(python) python/create_init.py $(python_output_dir)

$(rust_src_dir)/%_stemmer.rs: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(rust_src_dir)
	./snowball $< -rust -o "$(rust_src_dir)/$*_stemmer"

$(go_src_main_dir)/stemwords/algorithms.go: go/stemwords/generate.go libstemmer/modules.txt
	@echo "Generating algorithms.go"
	@cd go/stemwords && go generate

$(go_src_dir)/%_stemmer.go: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(go_src_dir)/$*
	./snowball $< -go -o "$(go_src_dir)/$*/$*_stemmer" -gop $*
	$(gofmt) -s -w $(go_src_dir)/$*/$*_stemmer.go

$(js_output_dir)/%-stemmer.js: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(js_output_dir)
	./snowball $< -js=global -o "$(js_output_dir)/$*-stemmer"

$(js_output_dir)/base-stemmer.js: $(js_runtime_dir)/base-stemmer.js
	@mkdir -p $(js_output_dir)
	cp $< $@
	echo "\nglobalThis.BaseStemmer = BaseStemmer;" >> $@

$(js_output_dir)/%-stemmer.esm.js: algorithms/%.sbl snowball$(EXEEXT)
	@mkdir -p $(js_output_dir)
	./snowball $< -js=esm -o "$(js_output_dir)/$*-stemmer"

$(js_output_dir)/base-stemmer.esm.js: $(js_runtime_dir)/base-stemmer.js
	@mkdir -p $(js_output_dir)
	cp $< $@
	echo "\nexport { BaseStemmer };" >> $@

$(ada_src_dir)/stemmer-%.ads: algorithms/%.sbl snowball
	@mkdir -p $(ada_src_dir)
	./snowball $< -ada -P $* -o "$(ada_src_dir)/stemmer-$*"

.PHONY: dist dist_snowball dist_libstemmer_c dist_libstemmer_csharp dist_libstemmer_java dist_libstemmer_js dist_libstemmer_python

# Make a full source distribution
dist: dist_snowball dist_libstemmer_c dist_libstemmer_csharp dist_libstemmer_java dist_libstemmer_js dist_libstemmer_python

# Make a distribution of all the sources involved in snowball
dist_snowball: $(COMPILER_SOURCES) $(COMPILER_HEADERS) \
	    $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
	    $(LIBSTEMMER_SOURCES) \
	    $(LIBSTEMMER_UTF8_SOURCES) \
            $(LIBSTEMMER_HEADERS) \
	    $(LIBSTEMMER_EXTRA) \
	    $(ALL_ALGORITHM_FILES) $(STEMWORDS_SOURCES) $(STEMTEST_SOURCES) \
	    $(COMMON_FILES) \
	    GNUmakefile README.rst doc/TODO libstemmer/mkmodules.pl
	destname=snowball-$(SNOWBALL_VERSION); \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}$(tarball_ext) && \
	for file in $^; do \
	  dir=`dirname $$file` && \
	  mkdir -p $${dest}/$${dir} && \
	  cp -a $${file} $${dest}/$${dir} || exit 1 ; \
	done && \
	(cd dist && tar zcf $${destname}$(tarball_ext) $${destname}) && \
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
	    $(COMMON_FILES) \
            libstemmer/mkinc.mak \
            libstemmer/mkinc_utf8.mak
	destname=libstemmer_c-$(SNOWBALL_VERSION); \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}$(tarball_ext) && \
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
	 echo "README.rst" >> MANIFEST && \
	 ls $(c_src_dir)/*.c $(c_src_dir)/*.h >> MANIFEST && \
	 ls runtime/*.c runtime/*.h >> MANIFEST && \
	 ls libstemmer/*.c libstemmer/*.h >> MANIFEST && \
	 ls include/*.h >> MANIFEST) && \
        cp -a libstemmer/mkinc.mak libstemmer/mkinc_utf8.mak $${dest}/ && \
	cp -a $(COMMON_FILES) $${dest} && \
	echo 'include mkinc.mak' >> $${dest}/Makefile && \
	echo 'ifeq ($$(OS),Windows_NT)' >> $${dest}/Makefile && \
	echo 'EXEEXT=.exe' >> $${dest}/Makefile && \
	echo 'endif' >> $${dest}/Makefile && \
	echo 'CFLAGS=-O2' >> $${dest}/Makefile && \
	echo 'CPPFLAGS=-Iinclude' >> $${dest}/Makefile && \
	echo 'all: libstemmer.a stemwords$$(EXEEXT)' >> $${dest}/Makefile && \
	echo 'libstemmer.a: $$(snowball_sources:.c=.o)' >> $${dest}/Makefile && \
	echo '	$$(AR) -cru $$@ $$^' >> $${dest}/Makefile && \
	echo 'stemwords$$(EXEEXT): examples/stemwords.o libstemmer.a' >> $${dest}/Makefile && \
	echo '	$$(CC) $$(CFLAGS) -o $$@ $$^' >> $${dest}/Makefile && \
	echo 'clean:' >> $${dest}/Makefile && \
	echo '	rm -f stemwords$$(EXEEXT) libstemmer.a *.o $(c_src_dir)/*.o examples/*.o runtime/*.o libstemmer/*.o' >> $${dest}/Makefile && \
	(cd dist && tar zcf $${destname}$(tarball_ext) $${destname}) && \
	rm -rf $${dest}

# Make a distribution of all the sources required to compile the Java library.
dist_libstemmer_java: $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
	    $(COMMON_FILES) \
            $(LIBSTEMMER_EXTRA) \
	    $(JAVA_SOURCES)
	destname=libstemmer_java-$(SNOWBALL_VERSION); \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}$(tarball_ext) && \
	mkdir -p $${dest} && \
	cp -a doc/libstemmer_java_README $${dest}/README && \
	mkdir -p $${dest}/$(java_src_dir) && \
	cp -a $(JAVA_SOURCES) $${dest}/$(java_src_dir) && \
	mkdir -p $${dest}/$(java_src_main_dir) && \
	cp -a $(JAVARUNTIME_SOURCES) $${dest}/$(java_src_main_dir) && \
	cp -a $(COMMON_FILES) $${dest} && \
	(cd $${dest} && \
	 echo "README" >> MANIFEST && \
	 ls $(java_src_dir)/*.java >> MANIFEST && \
	 ls $(java_src_main_dir)/*.java >> MANIFEST) && \
	(cd dist && tar zcf $${destname}$(tarball_ext) $${destname}) && \
	rm -rf $${dest}

# Make a distribution of all the sources required to compile the C# library.
dist_libstemmer_csharp: $(RUNTIME_SOURCES) $(RUNTIME_HEADERS) \
	    $(COMMON_FILES) \
            $(LIBSTEMMER_EXTRA) \
	    $(CSHARP_SOURCES)
	destname=libstemmer_csharp-$(SNOWBALL_VERSION); \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}$(tarball_ext) && \
	mkdir -p $${dest} && \
	cp -a doc/libstemmer_csharp_README $${dest}/README && \
	mkdir -p $${dest}/$(csharp_src_dir) && \
	cp -a $(CSHARP_SOURCES) $${dest}/$(csharp_src_dir) && \
	mkdir -p $${dest}/$(csharp_src_main_dir) && \
	cp -a $(CSHARP_RUNTIME_SOURCES) $${dest}/$(csharp_src_main_dir) && \
	mkdir -p $${dest}/$(csharp_sample_dir) && \
	cp -a $(CSHARP_STEMWORDS_SOURCES) $${dest}/$(csharp_sample_dir) && \
	cp -a $(COMMON_FILES) $${dest} && \
	(cd dist && tar zcf $${destname}$(tarball_ext) $${destname}) && \
	rm -rf $${dest}

dist_libstemmer_python: $(PYTHON_SOURCES) $(COMMON_FILES)
	destname=snowballstemmer-$(SNOWBALL_VERSION); \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}$(tarball_ext) && \
	mkdir -p $${dest} && \
	mkdir -p $${dest}/src/$(python_runtime_dir) && \
	mkdir -p $${dest}/src/$(python_sample_dir) && \
	cp libstemmer/modules.txt $${dest} && \
	cp doc/libstemmer_python_README $${dest}/README.rst && \
	cp -a $(PYTHON_SOURCES) $${dest}/src/$(python_runtime_dir) && \
	cp -a $(PYTHON_SAMPLE_SOURCES) $${dest}/src/$(python_sample_dir) && \
	cp -a $(PYTHON_RUNTIME_SOURCES) $${dest}/src/$(python_runtime_dir) && \
	cp -a $(COMMON_FILES) $(PYTHON_PACKAGE_FILES) $${dest} && \
	(cd $${dest} && $(python) -m build && cp dist/*.tar.gz dist/*.whl ..) && \
	rm -rf $${dest}

dist_libstemmer_js: $(JS_SOURCES) $(COMMON_FILES)
	destname=jsstemmer-$(SNOWBALL_VERSION); \
	dest=dist/$${destname}; \
	rm -rf $${dest} && \
	rm -f $${dest}$(tarball_ext) && \
	mkdir -p $${dest} && \
	mkdir -p $${dest}/$(js_runtime_dir) && \
	mkdir -p $${dest}/$(js_sample_dir) && \
	cp -a doc/libstemmer_js_README $${dest}/README.rst && \
	cp -a $(COMMON_FILES) $${dest} && \
	cp -a $(JS_RUNTIME_SOURCES) $${dest}/$(js_runtime_dir) && \
	cp -a $(JS_SAMPLE_SOURCES) $${dest}/$(js_sample_dir) && \
	cp -a $(JS_SOURCES) $${dest}/$(js_runtime_dir) && \
	(cd $${dest} && \
	 ls README.rst $(COMMON_FILES) $(js_runtime_dir)/*.js $(js_sample_dir)/*.js > MANIFEST) && \
	(cd dist && tar zcf $${destname}$(tarball_ext) $${destname}) && \
	rm -rf $${dest}

###############################################################################
# C
###############################################################################

.PHONY: check check_stemtest check_utf8 check_iso_8859_1 check_iso_8859_2 check_koi8r

check: check_stemtest check_utf8 check_iso_8859_1 check_iso_8859_2 check_koi8r

check_stemtest: stemtest$(EXEEXT)
	./stemtest

check_utf8: $(libstemmer_algorithms:%=check_utf8_%)

check_iso_8859_1: $(ISO_8859_1_algorithms:%=check_iso_8859_1_%)

check_iso_8859_2: $(ISO_8859_2_algorithms:%=check_iso_8859_2_%)

check_koi8r: $(KOI8_R_algorithms:%=check_koi8r_%)

check_utf8_%: $(STEMMING_DATA)/% stemwords$(EXEEXT)
	@echo "Checking output of $* stemmer with UTF-8"
	@if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz'|./stemwords$(EXEEXT) -c UTF_8 -l $* -o tmp.txt; \
	else \
	  ./stemwords$(EXEEXT) -c UTF_8 -l $* -i $</voc.txt -o tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

check_iso_8859_1_%: $(STEMMING_DATA)/% stemwords$(EXEEXT)
	@echo "Checking output of $* stemmer with ISO_8859_1"
	@$(ICONV) -f UTF-8 -t ISO-8859-1 '$</voc.txt' |\
	    ./stemwords -c ISO_8859_1 -l $* -o tmp.txt
	@$(ICONV) -f UTF-8 -t ISO-8859-1 '$</output.txt' |\
	    $(DIFF) -u - tmp.txt
	@rm tmp.txt

check_iso_8859_2_%: $(STEMMING_DATA)/% stemwords$(EXEEXT)
	@echo "Checking output of $* stemmer with ISO_8859_2"
	@$(ICONV) -f UTF-8 -t ISO-8859-2 '$</voc.txt' |\
	    ./stemwords -c ISO_8859_2 -l $* -o tmp.txt
	@$(ICONV) -f UTF-8 -t ISO-8859-2 '$</output.txt' |\
	    $(DIFF) -u - tmp.txt
	@rm tmp.txt

check_koi8r_%: $(STEMMING_DATA)/% stemwords$(EXEEXT)
	@echo "Checking output of $* stemmer with KOI8R"
	@$(ICONV) -f UTF-8 -t KOI8-R '$</voc.txt' |\
	    ./stemwords -c KOI8_R -l $* -o tmp.txt
	@$(ICONV) -f UTF-8 -t KOI8-R '$</output.txt' |\
	    $(DIFF) -u - tmp.txt
	@rm tmp.txt

###############################################################################
# Java
###############################################################################

.PHONY: java check_java do_check_java

.SUFFIXES: .class .java

java: $(JAVA_CLASSES) $(JAVA_RUNTIME_CLASSES)

.java.class:
	cd java && $(JAVAC) $(JAVACFLAGS) $(patsubst java/%,%,$<)

check_java: java
	$(MAKE) do_check_java

do_check_java: $(libstemmer_algorithms:%=check_java_%)

check_java_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer for Java"
	@cd java && if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' |\
	    $(JAVA) org/tartarus/snowball/TestApp $* -o $(PWD)/tmp.txt; \
	else \
	  $(JAVA) org/tartarus/snowball/TestApp $* $</voc.txt -o $(PWD)/tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

###############################################################################
# C#
###############################################################################

.PHONY: csharp check_csharp do_check_csharp

csharp: csharp_stemwords$(EXEEXT)

check_csharp: csharp
	$(MAKE) do_check_csharp

do_check_csharp: $(libstemmer_algorithms:%=check_csharp_%)

check_csharp_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer for C#"
	@if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' |\
	    $(MONO) csharp_stemwords$(EXEEXT) -l $* -i /dev/stdin -o tmp.txt; \
	else \
	  $(MONO) csharp_stemwords$(EXEEXT) -l $* -i $</voc.txt -o tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

###############################################################################
# Pascal
###############################################################################

.PHONY: pascal check_pascal do_check_pascal

pascal: pascal/stemwords

check_pascal: pascal
	$(MAKE) do_check_pascal

do_check_pascal: $(ISO_8859_1_algorithms:%=check_pascal_%)

check_pascal_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer with ISO_8859_1 for Pascal"
	@$(ICONV) -f UTF-8 -t ISO-8859-1 '$</voc.txt' |\
	    ./pascal/stemwords -l $* > tmp.txt
	@$(ICONV) -f UTF-8 -t ISO-8859-1 '$</output.txt' |\
	    $(DIFF) -u - tmp.txt
	@rm tmp.txt

###############################################################################
# Javascript
###############################################################################

.PHONY: js check_js do_check_js

js: $(JS_SOURCES)

check_js: js
	$(MAKE) do_check_js

do_check_js: $(libstemmer_algorithms:%=check_js_%)

check_js_%: export NODE_PATH=$(js_output_dir)
check_js_%: $(STEMMING_DATA)/%
	@echo "Checking output of $* stemmer for JS"
	@if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' > tmp.in; \
	  $(JSRUN) javascript/stemwords.js -l $* -i tmp.in -o tmp.txt; \
	  rm tmp.in; \
	else \
	  $(JSRUN) javascript/stemwords.js -l $* -i $</voc.txt -o tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

	@echo "Checking output of $* stemmer for JS (ESM)"
	@if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' > tmp.in; \
	  $(JSRUN) javascript/stemwords.js --esm -l $* -i tmp.in -o tmp.txt; \
	  rm tmp.in; \
	else \
	  $(JSRUN) javascript/stemwords.js --esm -l $* -i $</voc.txt -o tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

###############################################################################
# Rust
###############################################################################

.PHONY: rust check_rust do_check_rust

rust: $(RUST_SOURCES)

check_rust: rust
	$(MAKE) do_check_rust

do_check_rust: $(libstemmer_algorithms:%=check_rust_%)

check_rust_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer for Rust"
	@cd rust && if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' > tmp.in; \
	  $(cargo) run $(cargoflags) -- -l $* -i tmp.in -o $(PWD)/tmp.txt; \
	  rm tmp.in; \
	else \
	  $(cargo) run $(cargoflags) -- -l $* -i $</voc.txt -o $(PWD)/tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

###############################################################################
# Go
###############################################################################

.PHONY: go check_go do_check_go

go: $(GO_SOURCES)

check_go: go
	$(MAKE) do_check_go

do_check_go: $(libstemmer_algorithms:%=check_go_%)

check_go_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer for Go"
	@cd go && if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' > tmp.in; \
	  $(go) run $(goflags) -l $* -i tmp.in -o $(PWD)/tmp.txt; \
	  rm tmp.in; \
	else \
	  $(go) run $(goflags) -l $* -i $</voc.txt -o $(PWD)/tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

###############################################################################
# Python
###############################################################################

.PHONY: python check_python do_check_python

python: check_python_stemwords

check_python: python
	$(MAKE) $(libstemmer_algorithms:%=check_python_%)

check_python_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer for Python (THIN_FACTOR=$(THIN_FACTOR))"
	@cd python_check && if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' $(THIN_TEST_DATA) > tmp.in; \
	  $(python) stemwords.py -c utf8 -l $* -i tmp.in -o $(PWD)/tmp.txt; \
	  rm tmp.in; \
	else \
	  $(python) stemwords.py -c utf8 -l $* -i $</voc.txt -o $(PWD)/tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz' $(THIN_TEST_DATA)|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

check_python_stemwords: $(PYTHON_STEMWORDS_SOURCE) $(PYTHON_SOURCES)
	mkdir -p python_check
	mkdir -p python_check/snowballstemmer
	cp -a $(PYTHON_RUNTIME_SOURCES) python_check/snowballstemmer
	cp -a $(PYTHON_SOURCES) python_check/snowballstemmer
	cp -a $(PYTHON_STEMWORDS_SOURCE) python_check/

###############################################################################
# Ada
###############################################################################

.PHONY: ada check_ada do_check_ada

ada: ada/bin/stemwords

check_ada: ada
	$(MAKE) do_check_ada

do_check_ada: $(libstemmer_algorithms:%=check_ada_%)

check_ada_%: $(STEMMING_DATA_ABS)/%
	@echo "Checking output of $* stemmer for Ada"
	@cd ada && if test -f '$</voc.txt.gz' ; then \
	  gzip -dc '$</voc.txt.gz' > tmp.in; \
	  ./bin/stemwords $* tmp.in $(PWD)/tmp.txt; \
	  rm tmp.in; \
	else \
	  ./bin/stemwords $* $</voc.txt $(PWD)/tmp.txt; \
	fi
	@if test -f '$</output.txt.gz' ; then \
	  gzip -dc '$</output.txt.gz'|$(DIFF) -u - tmp.txt; \
	else \
	  $(DIFF) -u $</output.txt tmp.txt; \
	fi
	@rm tmp.txt

$(ada_src_dir)/stemmer-factory.ads $(ada_src_dir)/stemmer-factory.adb: ada/bin/generate
	cd $(ada_src_dir) && ../bin/generate $(libstemmer_algorithms)

ada/bin/generate:
	cd ada && $(gprbuild) -Pgenerate -p

ada/bin/stemwords: $(ADA_SOURCES)
	cd ada && $(gprbuild) -Pstemwords -p
