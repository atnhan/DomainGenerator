CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -I$(BOOST)

OBJS =		DomainGenerator.o

LIBS =		

BOOST =		/usr/local/boost_1_53_0

TARGET =	DomainGenerator

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	cp DomainGenerator ../Robust-FF/benchmark

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
