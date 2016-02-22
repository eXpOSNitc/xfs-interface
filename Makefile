DISK = disk

all: xfs-interface

xfs-interface: fileSystem.h fileSystem.c fileUtility.h fileUtility.h interface.h interface.c createDisk.h createDisk.c labels.c labels.h
	gcc -g fileSystem.c fileUtility.c labels.c interface.c createDisk.c -o xfs-interface -lreadline

clean:
	rm -rf $(DISK) *.o xfs-interface 2> /dev/null
