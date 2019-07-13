all:
	g++ -lkmlengine -lkmldom -lkmlbase gadmextract.cc -o gadmextract
	g++ geo2pxf.cc common.cc -o geo2pxf
