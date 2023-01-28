clean() {
    cd $1
    make clean
    rm -f allocation.c
    rm -f allocation.h
    rm -f inode.c
    rm -f inode.h
    rm -f makefile
    cd ..
}

clean load_example1
clean load_example2
clean load_example3

clean create_example1
clean create_example2
clean create_example3
