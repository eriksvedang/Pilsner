
(def fact (fn (x)
            (if (= x 1)
              1
              (* x (fact (- x 1))))))

;;(def b (fn (x) (do (break) (* x x))))

(def nums '(1 2 3 4 5))
(def chars '("a" "b" "c" "d" "e"))


(def fib (fn (n)
           (if (= 1 n)
             1
             (if (= 2 n)
               1
               (+ (fib (- n 1)) (fib (- n 2)))))))

(def ahaa (fn (_) (range 1 10000)))

(def aha (fn (n) (map ahaa (range 1 n))))

(def b (fn (x) (do (break)
		   (inc x))))

(def bf (fn (x) (do (println (* x x))
		    (break)
		    (println (* x x x))
		    (break)
		    (println (+ x 100)))))

(def p (fn () (do (println "hej")
		  (println "på")
		  (println "dig"))))

(def t (fn () (map inc '(5 10 15))))


;; (load "tests.lisp")

;; (println "")
;; (println "SHADE")
;; (println "")
;; (def glob 123)
;; (def shade (fn (glob) (* glob glob))) ;; should shadow global variable 'glob'
;; (print-code shade)

;; (println "")
;; (println "NOSHADE")
;; (println "")
;; (def blub 321)
;; (def noshade (fn () (* blub blub)))
;; (print-code noshade)


(def iter
    (fn (i)
	(do (println i)
	    (stack)
	    (iter (inc i)))))


(def iter-n
    (fn (n f)
	(do (f n)
	    (if (< 2 n)
		(iter-n (dec n) f)
		'done))))

(def plus +)

(def not-iter
    (fn (n)
	(if (< n 2)
	    1
	    (plus n (not-iter (dec n))))))

; (def u (fn (x) (* x uu)))

