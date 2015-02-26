

(def fib (fn [n]
           (if (= 1 n)
             1
             (if (= 2 n)
               1
               (+ (fib (- n 1)) (fib (- n 2)))))))

