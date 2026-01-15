CC = gcc
CFLAGS = -I inc -std=c11 -Wall -Werror -Wextra
GCOV_FLAGS = --coverage
LDLIBS = -lncursesw
LDLIBS_TEST = -lcheck -lsubunit -lm
OPEN_CMD = xdg-open

DIR_SRC_LOG = brick_game/tetris
DIR_SRC_GNU = gui/cli
DIR_SRC_TEST = test
DIR_BUILD = build
DIR_OBJ = build/obj
DIR_LIB = build/lib
DIR_BIN = build/bin
DIR_TEST_BUILD = test_build
DIR_TEST_BIN = test_build/bin
DIR_GCOV_RES = test_build/gcov_res

.PHONY: all, install, uninstall, clean, dvi, dist, test, gcov_report, clean, tetris

all: install tetris

install: $(DIR_BIN)/s21_tetris

uninstall:
	rm -rf $(DIR_BUILD)

clean: clean_doc clean_dist
	rm -rf *.txt debug_bin $(DIR_BUILD) $(DIR_TEST_BUILD)

clean_doc:
	rm -rf doxyDoc

clean_dist:
	rm -rf doxyDoc s21_tetris_dist.tar.gz

dvi:
	mkdir -p doxyDoc
	@doxygen Doxyfile
	$(OPEN_CMD) doxyDoc/html/

dist:
	tar -czf s21_tetris_dist.tar.gz brick_game gui images inc test Makefile Doxyfile

whatch_dist:
	tar -tzf s21_tetris_dist.tar.gz

test: $(DIR_SRC_TEST)/s21_test.c $(DIR_LIB)/s21_tetris_fsm.a
	mkdir -p $(DIR_TEST_BIN)
	$(CC) $(CFLAGS) $^ -o $(DIR_TEST_BIN)/s21_test $(LDLIBS_TEST)
	./$(DIR_TEST_BIN)/s21_test

gcov_report: $(DIR_GCOV_RES)/gcov_test
	./$(DIR_GCOV_RES)/gcov_test
	lcov \
		-t "gcov_test" \
		--base-directory . \
		-c \
		-d $(DIR_GCOV_RES) \
		--no-external \
		-o $(DIR_GCOV_RES)/report.info
	genhtml -o $(DIR_GCOV_RES)/html_report $(DIR_GCOV_RES)/report.info
	$(OPEN_CMD) $(DIR_GCOV_RES)/html_report/

tetris:
	./$(DIR_BIN)/s21_tetris

$(DIR_BIN)/s21_tetris: $(DIR_SRC_GNU)/frontend.c $(DIR_LIB)/s21_tetris_fsm.a
	mkdir -p $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(DIR_LIB)/s21_tetris_fsm.a: $(DIR_OBJ)/s21_tetris_fsm.o
	mkdir -p $(DIR_LIB)
	ar rcs $@ $^

$(DIR_OBJ)/s21_tetris_fsm.o: $(DIR_SRC_LOG)/fsm.c
	mkdir -p $(DIR_OBJ)
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDLIBS)

$(DIR_GCOV_RES)/gcov_test: $(DIR_SRC_TEST)/s21_test.c $(DIR_SRC_LOG)/fsm.c
	rm -rf $(DIR_GCOV_RES)
	mkdir -p $(DIR_GCOV_RES)
	$(CC) $(CFLAGS) $(GCOV_FLAGS) $^ -o $@ $(LDLIBS_TEST)


ALL_SRC_FILES = $(shell find . -name "*.c")

check_code:
	if [ ! -f ../materials/linters/.clang-format ]; then \
		echo "Error clang-format: config file .clang-format not found"; \
	else \
		clang-format -n -style=file:../materials/linters/.clang-format $(ALL_SRC_FILES) inc/*.h; \
	fi
	cppcheck -I inc --enable=all --check-level=exhaustive --suppress=missingIncludeSystem $(ALL_SRC_FILES) inc/*.h

stylize_code:
	@if [ ! -f ../materials/linters/.clang-format ]; then \
		echo "Error clang-format: file .clang-format not found"; \
	else \
		clang-format -i -style=file:../materials/linters/.clang-format $(ALL_SRC_FILES) inc/*.h; \
	fi

DEBUG_FLAGS = -fsanitize=address -fsanitize=undefined -fsanitize=unreachable

check_leaks: $(DIR_SRC_GNU)/frontend.c $(DIR_SRC_LOG)/fsm.c
	mkdir -p debug_bin $(DIR_TEST_BUILD)/debugger_res
	clang -I inc -o debug_bin/test_by_clang $^ $(DEBUG_FLAGS) $(LDLIBS)
	( sleep 2; echo "q" ) | timeout 10s ./debug_bin/test_by_clang 2>&1 | tail -20
	rm -f debug_bin/test_by_clang
	$(CC) -I inc -o debug_bin/test_by_gcc $^ $(DEBUG_FLAGS) $(LDLIBS)
	( sleep 2; echo "q" ) | timeout 10s ./debug_bin/test_by_gcc 2>&1 | tail -20
	rm -f debug_bin/test_by_gcc
	$(CC) $(CFLAGS) $^ -o debug_bin/test_by_valgrind $(LDLIBS)
	( sleep 2; echo "q" ) | timeout 10s \
		valgrind \
			--tool=memcheck \
			--leak-check=yes \
			./debug_bin/test_by_valgrind 2>&1 | tail -100
	rm -f debug_bin/test_by_valgrind
	$(CC) $(CFLAGS) $^ -o debug_bin/test_by_valgrind $(LDLIBS)
	( sleep 2; echo "q" ) | timeout 10s \
		valgrind \
			--leak-check=full \
			--show-leak-kinds=all \
			--track-origins=yes \
			--verbose \
			--log-file=$(DIR_TEST_BUILD)/debugger_res/valgrind_report.txt \
			--error-exitcode=1 \
			./debug_bin/test_by_valgrind 2>&1 | tail -20
	$(OPEN_CMD) $(DIR_TEST_BUILD)/debugger_res/
	rm -rf debug_bin

check_leaks_in_tests: $(DIR_SRC_TEST)/s21_test.c $(DIR_SRC_LOG)/fsm.c
	mkdir -p debug_bin $(DIR_TEST_BUILD)/debugger_res
	clang -I inc -o debug_bin/test_by_clang_for_tests $^ $(DEBUG_FLAGS) $(LDLIBS_TEST)
	( sleep 2; echo "q" ) | timeout 10s ./debug_bin/test_by_clang_for_tests 2>&1 | tail -20
	rm -f debug_bin/test_by_clang_for_tests
	$(CC) -I inc -o debug_bin/test_by_gcc_for_tests $^ $(DEBUG_FLAGS) $(LDLIBS_TEST)
	( sleep 2; echo "q" ) | timeout 10s ./debug_bin/test_by_gcc_for_tests 2>&1 | tail -20
	rm -f debug_bin/test_by_gcc_for_tests
	$(CC) $(CFLAGS) $^ -o debug_bin/test_by_valgrind_for_tests $(LDLIBS_TEST)
	( sleep 2; echo "q" ) | timeout 10s \
		valgrind \
			--tool=memcheck \
			--leak-check=yes \
			./debug_bin/test_by_valgrind_for_tests 2>&1 | tail -100
	rm -f debug_bin/test_by_valgrind_for_tests
	$(CC) $(CFLAGS) $^ -o debug_bin/test_by_valgrind_for_tests $(LDLIBS_TEST)
	( sleep 2; echo "q" ) | timeout 10s \
		valgrind \
			--leak-check=full \
			--show-leak-kinds=all \
			--track-origins=yes \
			--verbose \
			--log-file=$(DIR_TEST_BUILD)/debugger_res/valgrind_report_for_tests.txt \
			--error-exitcode=1 \
			./debug_bin/test_by_valgrind_for_tests 2>&1 | tail -20
	$(OPEN_CMD) $(DIR_TEST_BUILD)/debugger_res/
	rm -rf debug_bin