.phony all:
all: diskinfo disklist diskget

diskinfo: diskinfo.c
	gcc diskinfo.c -o diskinfo

disklist: disklist.c
	gcc disklist.c -o disklist

diskget: diskget.c
	gcc diskget.c -o diskget

.PHONY clean:
clean:
	-rm -rf *.o *.exe
