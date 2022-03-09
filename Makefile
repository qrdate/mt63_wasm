TARGET = mt63Wasm
BUILD_DIR = build
CXX = em++
LD = $(CXX)

SOURCES = \
src/mt63_wasm.cpp \
src/mt63/mt63base.cpp \
src/mt63/dsp.cpp

LIB_EXPORTS = \
_getSampleRate,\
_encodeString,\
_getBuffer,\
_initRx,\
_processAudio,\
_processAudioResample

CXXFLAGS = \
-std=c++11 \
-Werror \
-Wall \
-O3 \
-flto

LDFLAGS = \
-flto \
--emit-symbol-map \
--closure 1 \
-s STRICT \
-s MODULARIZE \
-s EXPORT_NAME="mt63"\
-s SINGLE_FILE \
-s ENVIRONMENT="web,node" \
-s NO_FILESYSTEM \
-s ALLOW_MEMORY_GROWTH \
-s EXPORTED_FUNCTIONS="$(LIB_EXPORTS)" \
-s EXPORTED_RUNTIME_METHODS=ccall,cwrap

# Generate a list of object files from their source files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(sort $(SOURCES:.cpp=.o))))
# Add the directories the source files are in to the search path
vpath %.cpp $(sort $(dir $(SOURCES)))

.PHONY: all
all: $(BUILD_DIR)/$(TARGET).js

# Make emcc automatically generate dependency information
CXXFLAGS += -MMD -MF"$(@:%.o=%.d)"

$(BUILD_DIR)/$(TARGET).js: $(OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

# Include autogenerated dependency info
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)
