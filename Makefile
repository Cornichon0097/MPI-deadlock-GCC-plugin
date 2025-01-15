# ================================= Variables ================================ #
# ---------------------------- Sources directories --------------------------- #
SRCDIR     = src
TESTSDIR   = tests
INCLUDEDIR = include

# ----------------------------- Build directories ---------------------------- #
OBJDIR = obj
BINDIR = bin

# -------------------------------- Compilers --------------------------------- #
CC     = gcc_1220
MPICC  = mpicc # export MPICH_CC="gcc_1220"
CFLAGS = #-Wall -Wextra -Wstrict-prototypes -Wunreachable-code -Werror -O3 -g

CXX = g++_1220

PLUGIN_FLAGS = -I`$(CC) -print-file-name=plugin`/include -I$(INCLUDEDIR) \
               -Wall -fPIC -fno-rtti -g -shared

# ---------------------------------- Linker ---------------------------------- #
LD      = $(CC)
LDFLAGS =

# ----------------------------------- Files ---------------------------------- #
PLUGIN = libmpiplugin.so

PLUGIN_SOURCE_FILES = $(SRCDIR)/plugin.cpp \
                      $(SRCDIR)/print.cpp \
                      $(SRCDIR)/cfgviz.cpp \
                      $(SRCDIR)/mpicoll.cpp \
                      $(SRCDIR)/frontier.cpp \
                      $(SRCDIR)/pragma.cpp

PLUGIN_INCLUDES_FILES = $(INCLUDEDIR)/print.h \
                        $(INCLUDEDIR)/cfgviz.h \
                        $(INCLUDEDIR)/mpicoll.h \
                        $(INCLUDEDIR)/frontier.h \
                        $(INCLUDEDIR)/pragma.h \
                        $(INCLUDEDIR)/MPI_collectives.def

TARGETS = $(BINDIR)/hw.out \
          $(BINDIR)/ok.out \
          $(BINDIR)/simple.out \
          $(BINDIR)/pragma.out \
          $(BINDIR)/bad.out

# ============================= Targets and rules ============================ #
# ------------------------------ Default target ------------------------------ #
all: $(PLUGIN)

.PHONY: all

# ------------------------------- Directory rule ----------------------------- #
$(BINDIR):
	mkdir -p $@

# -------------------------------- Plugin rule ------------------------------- #
$(PLUGIN): $(PLUGIN_SOURCE_FILES) $(PLUGIN_INCLUDES_FILES)
	$(CXX) $(PLUGIN_FLAGS) $(GMP_CFLAGS) -o $@ $(PLUGIN_SOURCE_FILES)

# ------------------------------- Tests rules -------------------------------- #
tests: $(TARGETS)

.PHONY: tests

$(BINDIR)/hw.out: $(TESTSDIR)/hw.c \
                  $(PLUGIN) \
                  $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ -fplugin=./$(PLUGIN) $<

$(BINDIR)/ok.out: $(TESTSDIR)/ok.c \
                  $(PLUGIN) \
                  $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ -fplugin=./$(PLUGIN) $<

$(BINDIR)/simple.out: $(TESTSDIR)/simple.c \
                      $(PLUGIN) \
                      $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ -fplugin=./$(PLUGIN) $<

$(BINDIR)/pragma.out: $(TESTSDIR)/pragma.c \
                      $(PLUGIN) \
                      $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ -fplugin=./$(PLUGIN) $<

$(BINDIR)/bad.out: $(TESTSDIR)/bad.c \
                   $(PLUGIN) \
                   $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ -fplugin=./$(PLUGIN) $<

# -------------------------------- Main rules -------------------------------- #
clean:
	rm -f $(PLUGIN)

mrproper: clean
	rm -rf $(BINDIR)

.PHONY: clean \
        mrproper
