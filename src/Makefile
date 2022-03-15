all: dir boot.bin loader.bin boot.img

dir:
	test -d ../out || mkdir ../out
	test -d ../out/mount || mkdir ../out/mount

loader.bin:
	nasm loader.asm -o ../out/loader.bin

boot.bin:
	nasm boot.asm -o ../out/boot.bin

boot.img:
	dd if=../out/boot.bin of=../out/boot.img bs=512 count=1 conv=notrunc
	mount ../out/boot.img ../out/mount/ -t vfat -o loop
	cp ../out/loader.bin ../out/mount/
	sync
	umount ../out/mount/

clean:
	rm -rf *.bin