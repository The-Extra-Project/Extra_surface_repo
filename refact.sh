# astyle --style=allman --keep-one-line-blocks --delete-empty-lines --suffix=none  \
#       ./src/core/*.[ch]pp ./src/core/*/*.[ch]pp ./src/core/*/*.[ch]pp \
#       ./services/wasure/include/*.[ch]pp ./services/wasure/src/lib/*.[ch]pp ./services/wasure/src/exe/*.[ch]pp  \
#       ./services/ddt/include/*.[ch]pp ./services/ddt/src/lib/*.[ch]pp ./services/ddt/src/exe/*.[ch]pp

emacs -nw  ./src/core/*.[ch]pp ./src/core/*/*.[ch]pp ./src/core/*/*.[ch]pp \
       ./services/wasure/include/*.[ch]pp ./services/wasure/src/lib/*.[ch]pp ./services/wasure/src/exe/*.[ch]pp  \
       ./services/ddt/include/*.[ch]pp ./services/ddt/src/lib/*.[ch]pp ./services/ddt/src/exe/*.[ch]pp 
