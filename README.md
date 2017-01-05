# xf2bf

Small C utility that finds single hex byte strings (0x43, a1, 0xdd, 9c, 0x0e etc) in an ASCII encoded text file
and produces a binary consisting of all those bytes.

Name means xf(hex file) 2(to) bf(binary file)

## Example

test_file.txt:

~~~~
 48 0x65 6c 6c 0x6f 0x20 57 6f 0x72 6c 0x64
~~~~

Running `xf2bf test_file.txt` produces the following file

test_file.txt.out:

~~~~
Hello World
~~~~

Each byte word can have the '0x' prefix or not. The only requirement is that the single hex byte strings
have to be seperated by whitespace. Something like this "0x430x49fd" wouldn't work. Also all the other words in the file
that aren't of the single hex byte string format are ignored, but the final binary file based on each string will still be created.

## Building

    make

## License

MIT License
