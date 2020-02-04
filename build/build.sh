if [ ! -d "build" ]
then
  echo Must run build.sh from base project directory
  exit 1
fi
if [ ! -d "src" ]
then
  echo Must run build.sh from base project directory
  exit 1
fi

mkdir -p out/obj
$HOME/opt/cross/bin/i686-elf-gcc -std=gnu99 -ffreestanding -g -c src/start.s -o out/obj/start.o
$HOME/opt/cross/bin/i686-elf-gcc -std=gnu99 -ffreestanding -g -c src/kernel.c -o out/obj/kernel.o
$HOME/opt/cross/bin/i686-elf-gcc -ffreestanding -nostdlib -g -T src/linker.ld out/obj/start.o out/obj/kernel.o -oout/mykernel.elf -lgcc
