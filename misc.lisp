
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
