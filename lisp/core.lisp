
(def id (fn (x) x))
(def comp (fn (f g) (fn (x) (f (g x)))))

(def inc (fn (x) (+ x 1)))
(def dec (fn (x) (- x 1)))

(def zip
  (fn (a b) (if (nil? a)
              ()
              (cons (list (first a) (first b))
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

(def iter-n
    (fn (n f)
	(do (f n)
	    (if (< 2 n)
		(iter-n (dec n) f)
		'done))))

;; Use local let bindings instead!!!
(def timing
  (fn (f) (do (def start-time (time))
              (f)
              (def end-time (time))
              (print "dt:")
              (println (- end-time start-time)))))

