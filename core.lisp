
(def id (fn (x) x))

(def inc (fn (x) (+ x 1)))
(def dec (fn (x) (- x 1)))

(def fact (fn (x)
	      (if (= x 1)
		  1
		  (* x (fact (- x 1))))))

;(def b (fn (x) (do (break) (* x x))))

(def nums '(1 2 3 4 5))
(def chars '("a" "b" "c" "d" "e"))

(def zip
     (fn (a b) (if (nil? a)
		   ()
		 (cons (cons (first a) (first b))
		       (zip (rest a) (rest b))))))

(def range (fn (start end)
	       (if (> start end)
		   ()
		   (cons start (range (inc start) end)))))

