
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
               (+ (fib (dec n)) (fib (- n 2)))))))

(def ahaa (fn (_) (range 1 10000)))

(def aha (fn (n) (map ahaa (range 1 n))))

(def bf (fn (x) (do (println (* x x))
		    (break)
		    (println (* x x x))
		    (break)
		    (println (+ x 100)))))

(def p (fn () (do (println "hej")
		  (println "på")
		  (println "dig"))))

(def a 140)



