
(def t1
     (fn () (timing (fn () (fib 25)))))
;; Takes about 1.55 sec


(def t2
     (fn () (timing (fn () (fib 27)))))
;; Takes about 4.08 sec


(def t3
     (fn () (timing (fn () (fib 29)))))
;; Takes about 11 sec

