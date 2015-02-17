
(def f (fn (x) (* x x x)))

(def higher (fn (x)
		(fn () (println x))))

