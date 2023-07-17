BUILD_DIR := build
LATEX_FILE := ../main.tex

default: build

build:
	@ mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) \
		&& pdflatex $(LATEX_FILE)
.PHONY: build

clean:
	@ rm -rf $(BUILD_DIR)
.PHONY: clean
