
(def f (fn (x) (* x x x)))

;; (def higher (fn (x) (fn () (println x))))

(def max (fn (a b) (if (< a b) b a)))

(def g (fn (one two three four) (+ one three)))

