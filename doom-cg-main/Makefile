CXX      := g++
CXXFLAGS := -g -O0 -Wall -Iinclude
# Bibliotecas do Windows (MinGW)
LDLIBS   := -lglew32 -lfreeglut -lopengl32 -lglu32

TARGET    := DoomLike.exe
BUILD_DIR := build

# --- LISTA DE ARQUIVOS ---
# Adicione aqui o caminho de todos os seus arquivos .cpp
SRCS      := main.cpp \
             src/Game.cpp \
             src/Renderer.cpp
# -------------------------

OBJS      := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDLIBS)

# Regra para compilar os .o e criar as pastas no Windows
$(BUILD_DIR)/%.o: %.cpp
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	cd $(BUILD_DIR) && ./$(TARGET)

clean:
	@if exist $(BUILD_DIR) rd /s /q $(BUILD_DIR)