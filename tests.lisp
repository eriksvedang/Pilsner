
(def t1
     (fn () (timing (fn () (fib 25)))))
;; Takes about 1.55 sec
;; With op-codes: 0.606

(def t2
     (fn () (timing (fn () (fib 27)))))
;; Takes about 4.08 sec
;; With op-codes: 1.585

(def t3
     (fn () (timing (fn () (fib 29)))))
;; Takes about 11 sec
;; With op-codes: 4.148

;; ➜ (range 1 10)
;; + 272 Obj:s



