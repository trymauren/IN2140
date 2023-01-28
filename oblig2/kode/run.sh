link() {
    cd $1
    ln -fs ../allocation.c allocation.c
    ln -fs ../allocation.h allocation.h
    ln -fs ../inode.c inode.c
    ln -fs ../inode.h inode.h
    ln -fs ../makefile makefile
    cd ..
}

compile() {
    cd $1
    make $2
    cd ..
}

run() {
    cd $1
    valgrind --track-origins=yes --malloc-fill=0x40 --free-fill=0x23 --leak-check=full --show-leak-kinds=all -s ./$2
    cd ..
}

link load_example1
link load_example2
link load_example3

compile load_example1 load
compile load_example2 load
compile load_example3 load

run load_example1 load
run load_example2 load
run load_example3 load


link create_example1
link create_example2
link create_example3

compile create_example1 create
compile create_example2 create
compile create_example3 create

run create_example1 create
run create_example2 create
run create_example3 create
