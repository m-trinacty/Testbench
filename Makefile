TARGET ?= oDriveApp
CC ?= g++
CFLAGS = -g -Wall -O0
LDFLAGS = -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -fstack-protector-strong -Wl,-z,relro,-z,now
LIBS ?= -pthread

.PHONY: clean all default

default: $(TARGET)
all: default

debug: CXXFLAGS = -DDEBUG -g -Wall -O0
debug: CCFLAGS = -DDEBUG -g  -Wall -O0
debug: all

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	@echo 'Makefile'
all: Makefile $(TARGET)
	@echo 'All'
first: all
	@echo 'Install'
install_target: first
	-install -d $(DESTDIR)/$(PREFIX)/usr/local/bin
	-install -m 4755 -p $(INSTALL_ROOT)$(TARGET) $(DESTDIR)/$(PREFIX)/usr/local/bin/$(TARGET)
	@echo 'Install finished'
install: install_target
