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

qemu-system-i386 -kernel out/mykernel.elf
