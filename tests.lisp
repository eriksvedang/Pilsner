
(def t1
     (fn () (timing (fn () (fib 25)))))
;; Takes about 1.55 sec
;; NOW: 0.606

(def t2
     (fn () (timing (fn () (fib 27)))))
;; Takes about 4.08 sec
;; NOW: 1.585

(def t3
     (fn () (timing (fn () (fib 29)))))
;; Takes about 11 sec
;; NOW: 4.148

