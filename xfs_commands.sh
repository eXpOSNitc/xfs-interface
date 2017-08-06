./xfs-interface load --os ../spl/progs/os_startup.xsm 
./xfs-interface load --int=disk ../spl/progs/disk_int.xsm
./xfs-interface load --exhandler ../spl/progs/exception.xsm 
./xfs-interface load --int=4 ../spl/progs/int4.xsm 
./xfs-interface load --int=5 ../spl/progs/int5.xsm
./xfs-interface load --int=6 ../spl/progs/int6.xsm
./xfs-interface load --int=7 ../spl/progs/int7.xsm
./xfs-interface load --int=8 ../spl/progs/int8.xsm
./xfs-interface load --int=9 ../spl/progs/int9.xsm
./xfs-interface load --int=10 ../spl/progs/int10.xsm
./xfs-interface load --int=15 ../spl/progs/int15.xsm
./xfs-interface load --int=timer ../spl/progs/timer.xsm 
./xfs-interface load --module 0 ../spl/progs/ResManager.xsm 
./xfs-interface load --module 1 ../spl/progs/process_manager.xsm 
./xfs-interface load --module 2 ../spl/progs/mem_manage.xsm 
./xfs-interface load --module 3 ../spl/progs/BufManager.xsm
./xfs-interface load --module 4 ../spl/progs/DManager.xsm 
./xfs-interface load --module 5 ../spl/progs/scheduler.xsm
./xfs-interface load --idle ../expl/idle.xsm 
./xfs-interface load --init ../expl/lseek.xsm
