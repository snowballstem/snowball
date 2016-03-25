Pod::Spec.new do |s|
	s.name = "snowball"
	s.version = "2014.11.10"
	s.summary = "Snowball is a small string processing language designed for creating stemming algorithms for use in Information Retrieval."
	s.homepage = "http://snowball.tartarus.org"
	s.authors = "Martin Porter", "Richard Boulton"
	s.license = { :type => 'BSD', :text => <<-LICENSE
	All the software given out on this Snowball site is covered by the BSD License (see http://www.opensource.org/licenses/bsd-license.html ), with Copyright (c) 2001, Dr Martin Porter, and (for the Java developments) Copyright (c) 2002, Richard Boulton.

Essentially, all this means is that you can do what you like with the code, except claim another Copyright for it, or claim that it is issued under a different license. The software is also issued without warranties, which means that if anyone suffers through its use, they cannot come back and sue you. You also have to alert anyone to whom you give the Snowball software to the fact that it is covered by the BSD license.

We have not bothered to insert the licensing arrangement into the text of the Snowball software."
LICENSE
    }
	s.source = { http: "http://snowball.tartarus.org/dist/libstemmer_c.tgz" }
	s.prepare_command = 'make'
	s.source_files = 'libstemmer/libstemmer.c', 'libstemmer/modules.h', 'include/*.h', 'src_c/*.{h,c}', 'runtime/*.{h,c}', 'CocoaPods/snowball-umbrella.h'
	s.preserve_paths = 'libstemmer/*', 'src_c/*'
    s.public_header_files = 'include/*.h', 'CocoaPods/snowball-umbrella.h'
	s.xcconfig = { 'OTHER_CFLAGS' => '$(inherited) -DSNOWBALL_FRAMEWORK' }
	s.requires_arc = false
	
	s.module_name = 'snowball'
	s.module_map = 'CocoaPods/snowball.modulemap'
end