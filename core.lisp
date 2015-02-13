
(def id (fn (x) x))

(def inc (fn (x) (+ x 1)))
(def dec (fn (x) (- x 1)))

(def me "erik")
(def you "marie")

(def b (fn (x) (do (break) (* x x))))

(def fact (fn (x) (if (= x 1) 1 (* x (fact (- x 1))))))



