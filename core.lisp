
(def id (fn (x) x))
(def comp (fn (f g) (fn (x) (f (g x)))))

(def inc (fn (x) (+ x 1)))
(def dec (fn (x) (- x 1)))

(def zip
  (fn (a b) (if (nil? a)
              ()
              (cons (cons (first a) (first b))
                    (zip (rest a) (rest b))))))

(def range
  (fn (start end)
    (if (> start end)
      ()
      (cons start (range (inc start) end)))))

(def map
  (fn (f xs)
    (if (nil? xs)
      ()
      (cons (f (first xs)) (map f (rest xs))))))

(def reduce
  (fn (f x xs)
    (if (nil? xs)
      x
      (reduce f (f x (first xs)) (rest xs)))))

(def repeat
     (fn (n item)
	 (if (= 0 n)
	     ()
	   (cons item (repeat (dec n) item)))))

