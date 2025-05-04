(list
 (cons "plinth"
       (list (cons 'project-type          "c-library-shared")
             (cons 'source-directory      "src")
             (cons 'compile-options       (list "-fPIC" "-g" "-O0"))
             (cons 'ceedling-options      (list "-fPIC" "-g" "-O0"))
             (cons 'version               "0.0.1")
             (cons 'actions               (list "version" "clean" "compile" "link" "publish"))))
 )
