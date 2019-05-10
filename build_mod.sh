sudo rmmod src/mod/lepmod.ko
make
sudo insmod src/mod/lepmod.ko
#gcc src/mod/test.c -osrc/mod/test.out
