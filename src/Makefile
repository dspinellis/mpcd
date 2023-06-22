PREFIX ?= /usr/local
BINPREFIX ?= "$(PREFIX)/bin"
MANPREFIX ?= "$(PREFIX)/share/man/man1"

# All warnings, treat warnings as errors, generate dependencies in .d files
# offer C++11 features
CXXFLAGS=-Wall -Werror -MD -std=c++11

ifdef DEBUG
LDFLAGS=-g
CXXFLAGS+=-g -O0 -D_GLIBCXX_ASSERTIONS
else
CXXFLAGS+=-O2
endif

TEST_FILES=$(wildcard *Test.h)

all: mpcd


OBJS=TokenContainer.o CloneDetector.o

UnitTests: UnitTests.o $(OBJS)
	$(CXX) $(LDFLAGS) UnitTests.o $(OBJS) -lcppunit -o $@

UnitTests.o: $(TEST_FILES)

test: UnitTests
	./UnitTests

mpcd: $(OBJS) mpcd.o
	$(CXX) $(LDFLAGS) mpcd.o $(OBJS) -o $@

# Create a PDF version of the manual page
mpcd.pdf: mpcd.1
	groff -man -Tps $?| ps2pdf - $@

install: all
	@mkdir -p $(DESTDIR)$(MANPREFIX)
	@mkdir -p $(DESTDIR)$(BINPREFIX)
	install mpcd $(DESTDIR)$(BINPREFIX)/
	install -m 644 mpcd.1 $(DESTDIR)$(MANPREFIX)/

clean:
	rm -f *.o *.d *.exe mpcd UnitTests Token.h Keyword.h

# Pull-in dependencies generated with -MD
-include $(OBJS:.o=.d)
