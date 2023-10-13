.PHONY: mlfq_s
mlfq_s:
	./test_single.sh ./test_cases/case_501.txt mlfq g


.PHONY: mlfq
mlfq:
	./test_all.sh ./test_cases mlfq

.PHONY: sjf_s
sfj_s:
	./test_single.sh ./test_cases/case_501.txt sjf g

.PHONY: sjf
sjf:
	./test_all.sh ./test_cases sjf


.PHONY: stcf_s
stcf_s:
	./test_single.sh ./test_cases/case_501.txt stcf g

.PHONY: stcf
stcf:
	./test_all.sh ./test_cases stcf

.PHONY: rr_s
rr_s:
	./test_single.sh ./test_cases/case_501.txt rr g

.PHONY: rr
rr:
	./test_all.sh ./test_cases rr