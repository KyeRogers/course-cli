.PHONY: tests build clean help
help:
	@echo "Available commands:"
	@echo "  make test   - Run all unit tests"
	@echo "  make build     - Compile the app"
	@echo "  make clean   - Remove cached files and logs"

build: 
	g++ -Iinclude -std=c++20 -o course-cli src/main.cpp src/assignment.cpp src/manager.cpp src/course.cpp

tests:
	@g++ -Iinclude -std=c++20 -o assignment-tests tests/assignment_doctest.cpp src/assignment.cpp
	./assignment-tests
	@rm assignment-tests
	@g++ -Iinclude -std=c++20 -o manager-tests tests/manager_doctest.cpp src/manager.cpp src/assignment.cpp src/course.cpp
	./manager-tests
	@rm manager-tests


clean:
	@echo "Cleaning up..."
	rm course-cli
	@echo "Clean complete."

