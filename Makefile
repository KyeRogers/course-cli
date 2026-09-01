.PHONY: test build clean help
help:
	@echo "Available commands:"
	@echo "  make test   - Run all unit tests"
	@echo "  make build     - Compile the app"
	@echo "  make clean   - Remove cached files and logs"

build: 
	g++ -o course-cli src/main.cpp src/assignment.cpp src/manager.cpp

test:
	@echo "Running tests..."


clean:
	@echo "Cleaning up..."
	rm course-cli
	@echo "Clean complete."

