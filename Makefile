all: boot.bin loader.bin

loader.bin:
	nasm src/loader.asm -o src/loader.bin

boot.bin:
	nasm src/boot.asm -o src/boot.bin

clean:
	rm -rf src/*.bin