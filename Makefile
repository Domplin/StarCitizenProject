# Compiler and standard
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Paths for Include headers and Libraries
INCLUDES = -I"C:/msys64/ucrt64/include" -I"include"
LDFLAGS = -L"C:/msys64/ucrt64/lib"

# Libraries to link against
LIBS = -lgdi32 -ltesseract -lleptonica

# Target executable name
TARGET = capture.exe

# Source files
SRCS = src/main.cpp \
       src/ocr_engine.cpp \
       src/regionSelector.cpp \
       src/resource_matcher.cpp \
       src/screen_capture.cpp

# --- Build Rules ---

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LIBS)

# Clean target for Windows environments
.PHONY: clean
clean:
	@if exist $(TARGET) del /Q $(TARGET)