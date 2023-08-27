.PHONY: clean All

All:
	@echo ----------Building project:[ A3P1 - Release ]----------
	@make -f "Makefile.A3P1"
	@echo ----------Building project:[ A3P3 - Release ]----------
	@make -f "Makefile.A3P3"
clean:
	@echo ----------Cleaning project:[ A3P1 - Release ]----------
	@make -f "Makefile.A3P1" clean
	@echo ----------Cleaning project:[ A3P3 - Release ]----------
	@make -f "Makefile.A3P3" clean

