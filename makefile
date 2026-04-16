.PHONY: help
help:
	grep -v '^\.PHONY: ' makefile

.PHONY: benchmark-sv-baseline
benchmark-sv-baseline:
	(echo -n "# " ; date) >> benchmarks/sv-baseline.txt
	echo 100 1000 1 | python sv-baseline/benchmark.py | tee -a benchmarks/sv-baseline.txt
