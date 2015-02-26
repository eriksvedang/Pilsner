
(def fact (fn (x)
            (if (= x 1)
              1
              (* x (fact (- x 1))))))

(def fib (fn (n)
           (if (= 1 n)
             1
             (if (= 2 n)
               1
               (+ (fib (- n 1)) (fib (- n 2)))))))

(def nums '(1 2 3 4 5))
(def chars '("a" "b" "c" "d" "e"))

(def b (fn (x) (do (break)
		   (inc x))))

(def bb (fn (x) (do (println (* x x))
		    (break)
		    (println (* x x x))
		    (break)
		    (println (+ x 100)))))

(def p (fn () (do (println "hej")
		  (println "på")
		  (println "dig"))))

(def iter
    (fn (i)
	(do (println i)
	    (stack)
	    (iter (inc i)))))

(def plus +)

(def not-iter
    (fn (n)
	(if (< n 2)
	    1
	    (plus n (not-iter (dec n))))))

(def capture
    (fn (x)
	(fn () x)))

(def cap-1 (capture 450))
(def cap-2 (capture 666))

;; (def undf (fn () (* u u))) ;; using the undefined variable u


