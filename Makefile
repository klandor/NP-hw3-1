all: hw3-1.cgi

hw3-1.cgi: hw3-1.o
	$(CXX) hw3-1.o -o $@

install: hw3-1.cgi
	cp hw3-1.cgi ~pldiao/public_html/cgi-bin/
	cp hw3-1.html ~pldiao/public_html/

push:
	git push github master

pull:
	git pull github master

.PHONY: push pull