;; all.lsp -- load and play all pmorales demos/examples
;;
;; Roger Dannenberg, Aug 2012

(setf original-dir (setdir "."))
(setdir (current-path))
(setf working-dir (setdir "."))
(format t "In pmorales/all.lsp, set working directory to ~A~%" working-dir)
(load "a4")
(shiver-demo)
(load "a5")
(whiny-demo)
(load "a6")
(callas-demo)
(load "b1")
(play (gong-1))
(play (gong-2))
(gong-3-demo)
(load "b2")
(st-sac-demo)
(load "b3")
(risset-bell-demo)
; note: no b4 exists
(load "b5")
(starship-demo)
; note: no b6 exists
(load "b7")
(tibetan-demo)
(load "b8")
(risset-drum-demo)
(load "b9")
(risset-endless-demo)
(load "buzz")
(buzz-test)
(load "c1")
(tenny-demo)
(load "d1")
(load "e2")
(chowning-fm-demo)
(load "ks")
(load "partial")
(bell-demo)
(load "phm")
(setdir original-dir)
(format t "In pmorales/all.lsp, restored working directory to ~A~%" (setdir "."))