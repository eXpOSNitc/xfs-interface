DISK = disk

all: xfs-interface

xfs-interface: fileSystem.h fileSystem.c diskUtility.h diskUtility.h interface.h interface.c inode.h inode.c labels.c labels.h exception.h exception.c
	gcc -g fileSystem.c diskUtility.c labels.c interface.c inode.c exception.c -o xfs-interface -lreadline

clean:
	rm -rf $(DISK) *.o xfs-interface 2> /dev/null
