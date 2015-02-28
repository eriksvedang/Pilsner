
(assert-eq "Simple map"
	   '(10 20 30)
	   (map (fn (x) (* x 10)) (list 1 2 3)))

(assert-eq "Fibonacci"
	   '(1 1 2 3 5 8 13)
	   (map fib (range 1 7)))

(assert-eq "Read and Eval"
	   '420
	   (eval (read "(* 42 10)")))



