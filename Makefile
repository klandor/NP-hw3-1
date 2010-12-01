all: hw3-1.cgi

hw3-1.cgi: hw3-1.o
	$(CXX) hw3-1.o -o $@

install: hw3-1.cgi
	cp hw3-1.cgi ~pldiao/public_html/cgi-bin/