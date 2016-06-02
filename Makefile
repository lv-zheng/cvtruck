RM = rm -f
CXXSTDFLAGS ?= -std=c++14
CXXFLAGS := $(CXXFLAGS) -Wall -Wextra $(CXXSTDFLAGS)
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_videoio -lboost_system

all: cvtruck

SRCS = \
	Control.cpp \
	getP.cpp \
	main.cpp \
	opencv_tmp2.cpp \
	route.cpp \
	stdafx.cpp \
	trans.cpp \
	truck.cpp

OBJS = $(SRCS:.cpp=.o)

ELFS = \
	cvtruck

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^ >> ./.depend

cvtruck: $(OBJS)
	$(CXX) $(LDFLAGS) -o cvtruck $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend
