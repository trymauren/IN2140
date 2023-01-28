load: allocation.c inode.c load_fs.c
	gcc $^ -o $@
create: create_fs.c inode.c allocation.c
	gcc $^ -o $@
clean:
	rm -f load create
