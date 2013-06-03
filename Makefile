all:
	cd src; make

clean:
	cd src; qmake; make clean
	rm src/Makefile
	rm -f bin/CQZXSpectrum
