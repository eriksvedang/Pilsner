
(def t1
    (fn () (timing (fn () (fib 25)))))

;; Takes about 1.55 sec
;; With op-codes: 0.606
;; With args as C-array: 0.31
;; With direct lookup of global variabels: 0.141
;; With args stored in stack frames instead of envs: 0.078

(def t2
     (fn () (timing (fn () (fib 27)))))
;; Takes about 4.08 sec
;; With op-codes: 1.585
;; With args as C-array: 0.813
;; With direct lookup of global variabels: 0.365
;; With args stored in stack frames instead of envs: 0.2

(def t3
     (fn () (timing (fn () (fib 29)))))
;; Takes about 11 sec
;; With op-codes: 4.148
;; With args as C-array: 2.14
;; With direct lookup of global variabels: 0.965
;; With args stored in stack frames instead of envs: 0.525
;; With proper lookup in closing envs: 0.64

;; (timing (fn () (fib 31)))
;; With direct lookup of global variabels: 2.485
;; With args stored in stack frames instead of envs: 1.37
;; Memory: 1023 MB
;; With better struct packing: 689MB
;; With args stored in stack frames instead of envs: 272 MB

;; ➜ (range 1 10)
;; + 272 Obj:s
;; With args as C-array: 179 Obj:s
;; With args stored in stack frames instead of envs: 20 Obj:s
;; With proper lookup in closing envs: 62 Obj:s


(def t (fn (n)
	   (do (def start (time))
	       (fib n)
	       (println (- (time) start)))))

