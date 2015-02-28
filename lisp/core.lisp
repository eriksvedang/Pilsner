
(def id (fn (x) x))
(def comp (fn (f g) (fn (x) (f (g x)))))

(def inc (fn (x) (+ x 1)))
(def dec (fn (x) (- x 1)))

(def even? (fn (x) (= 0 (mod x 2))))
(def odd? (fn (x) (not (even? x))))

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

(def replicate
     (fn (n item)
	 (if (= 0 n)
	     ()
	   (cons item (replicate (dec n) item)))))

(def iter-n
    (fn (n f)
	(do (f n)
	    (if (< 2 n)
		(iter-n (dec n) f)
		'done))))

(def loop
    (fn (f)
	(do (f)
	    (loop f))))

(def keep
    (fn (pred xs)
	(if (nil? xs)
	    nil
	    (if (pred (first xs))
		(cons (first xs) (keep pred (rest xs)))
		(keep pred (rest xs))))))

(def remove
    (fn (pred xs)
	(keep (fn (x) (not (pred x))) xs)))

;; Use local let bindings instead!!!
(def timing
  (fn (f) (do (def start-time (time))
              (f)
              (def end-time (time))
              (print "dt:")
              (println (- end-time start-time)))))

(def assert
    (fn (n pred)
	(if pred
	    (println (str "Test '" n "' passed."))
	    (println (str "TEST '" n "' FAILED!!!")))))

(def assert-eq
    (fn (n a b)
	(assert n (= a b))))


