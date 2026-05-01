.PHONY: help
help:
	grep -v '^\.PHONY: ' makefile

temp:
	mkdir temp

benchmarks:
	mkdir benchmarks

.PHONY: remove-sv-baseline
remove-sv-baseline:
	rm -f benchmarks/sv-baseline.txt

.PHONY: benchmark-sv-baseline
benchmark-sv-baseline: | benchmarks temp
	python sv-baseline/benchmark.py -f benchmarks/sv-baseline.txt

.PHONY: show-sv-baseline
show-sv-baseline:
	python sv-baseline/plot.py -f benchmarks/sv-baseline.txt
