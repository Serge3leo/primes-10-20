.PHONY: help
help:
	grep -v '^\.PHONY: ' makefile

.PHONY: connect
connect:
	ssh -X desktop -t tmux new-session -A -s p12 -c desk/stackoverflow/primes-10-20

.PHONY: suspend
suspend:
	ssh -t desktop systemctl suspend

.PHONY: clean
clean:
	rm -rf temp

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

.PHONY: remove-foxfox
remove-foxfox:
	rm -f benchmarks/foxfox.txt

.PHONY: benchmark-foxfox
benchmark-foxfox: | benchmarks temp
	python foxfox/benchmark.py -f benchmarks/foxfox.txt

.PHONY: show-foxfox
show-foxfox:
	python foxfox/plot.py -f benchmarks/foxfox.txt

.PHONY: check-foxfox
check-foxfox:
	cat benchmarks/foxfox.txt | python tools/check.py \
		-c1 "echo {} {} | python sv-sieve2/primes.py" \
		-c2 "echo {} {} | python foxfox/primes.py"

temp/pakuula: pakuula/primes.cc | temp
	g++ -O2 -o temp/pakuula pakuula/primes.cc -lcrypto

.PHONY: remove-pakuula
remove-pakuula:
	rm -f temp/pakuula
	rm -f benchmarks/pakuula.txt

.PHONY: benchmark-pakuula
benchmark-pakuula: temp/pakuula | benchmarks temp
	python pakuula/benchmark.py -f benchmarks/pakuula.txt

.PHONY: show-pakuula
show-pakuula:
	python pakuula/plot.py -f benchmarks/pakuula.txt

temp/pakuula-2: pakuula-2/primes128.cc | temp
	g++ -O2 -o temp/pakuula-2 -march=native -mbmi2 pakuula-2/primes128.cc

.PHONY: remove-pakuula-2
remove-pakuula-2:
	rm -f temp/pakuula-2
	rm -f benchmarks/pakuula-2.txt

.PHONY: benchmark-pakuula-2
benchmark-pakuula-2: temp/pakuula-2 | benchmarks temp
	python pakuula-2/benchmark.py -f benchmarks/pakuula-2.txt

.PHONY: show-pakuula-2
show-pakuula-2:
	python pakuula-2/plot.py -f benchmarks/pakuula-2.txt

