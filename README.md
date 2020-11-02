# Randomness mixers

For my key stretching scripts see the [Doubleslow](https://github.com/vstoykovbg/doubleslow) repository (the key stretching with the "seed extension" (also known as "extension word" and "passphrase") in the [BIP39 specification](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) is very weak, this is why I wrote the [Doubleslow](https://github.com/vstoykovbg/doubleslow) scripts).

Doublerandom is included in the [Cryptopup live distro](https://github.com/vstoykovbg/cryptopup).

## Simple script for creating BIP39 mnemonic

The script `make-seed-simple.py` mixes the randomness from the OS (`/dev/random` or `os.urandom`) with the user's input (randomness from dice, coins, decks of cards, etc.).

If you don't want to waste time to read and ascertain the more sophisticated scripts - this script is for you. 

## Randomness from the mouse

The script `mouse-seed-generator.py` uses the mouse movements as a source of randomnes to produce a BIP39 mnemonic. It needs [X](https://en.wikipedia.org/wiki/X_Window_System) to be running.

## Randomness from the sound input

The script `doublerandom-seed-generator.py` creates a BIP39 mnemonic by collecting randomness from the sound input and the `haveged`. Make sure you have `arecord` and `haveged` installed on your system. The script also uses random data entered by the user via the keyboard.

There is usually noise at the microphone input even if there is no microphone attached. In this case you need to increase the microphone input's volume to the max. Don't forget to check if the microphone input is muted (it must not be muted).

Of course, it works better with a microphone or a [noise generator](https://www.google.com/search?q=simple+noise+generator+circuit) attached to the microphone input.

You need to boost the volume if the signal from the microphone is weak. You can check the signal level with apps like `alsamixer`, `pavucontrol` and `audacity`.

Also you can check if the noise level is good by redirecting the output from arecord to aplay:

```
$ arecord -d 5 | aplay
```

You shuold hear the noise from the microphone in your headphones or speakers connected to the sound output.

You can use these commands to boost the microphone input's volume:

```
$ amixer sset 'Mic Boost' 100%
$ amixer sset 'Capture' 100%
```

On some systems it may be different - `'Mic Boost (+20 dB)'` instead of `'Mic Boost'` and `on` instead of `100%`:

```
$ amixer sset 'Mic Boost (+20dB)' on
```

The mixer `alsamixer` will show all available settings.

To "see" the noise you can use this command:

```
$ arecord -f cd -t raw -d 1 | hexdump | head -n 30
Recording raw data 'stdin' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo
0000000 ec72 27c5 ee36 283e ed03 2808 e63f 204c
0000010 de18 179d dc0e 15b6 da86 1102 dec2 1be7
0000020 eaba 25f5 fc97 3600 02ed 3fd8 ffc0 3b63
0000030 f7e8 32b1 f2db 2ba6 ed59 27c2 e92f 2690
0000040 e74e 20cb eb58 22fb f02a 2bec f383 3091
0000050 f5a6 2ed9 f2bb 2ce3 f317 2dba f26c 2d0f
0000060 f076 27bf ed9e 25bc ec26 26ee ea6e 21ab
0000070 e76e 2098 e755 2004 e6fc 1d39 e464 1e85
0000080 e344 1bbe e69d 21de e9d4 2301 ecd7 273e
0000090 f179 2eb0 f295 2bb0 f440 2e94 f130 2d8a
00000a0 ebbe 2679 ed59 2663 ef79 2a18 f650 316b
00000b0 fc61 3678 0339 3c99 05d1 4037 02d7 3c86
00000c0 0081 3c21 007b 3aaf f80e 32dd f101 29fc
00000d0 f6f2 3213 fcef 3629 fe19 3839 0520 410e
00000e0 09a2 40ac 007e 39b8 fb51 3530 facf 355c
00000f0 f783 31b4 f77a 3165 f427 3071 ee40 2aff
0000100 eede 2899 ef1a 2b48 f1f7 30cd f637 2f65
0000110 f70b 31a1 f6e2 3242 f883 31c7 010c 3c60
0000120 04ed 40e8 fe09 3ac5 faa3 3769 f627 30ed
0000130 f176 2a6a eabe 22f7 e180 1a5f d995 12df
0000140 dd2a 1645 e487 1cd1 ef20 2679 f844 31ca
0000150 faa0 33fd fa9a 367c f811 355c f307 2e9a
0000160 f186 2a87 f0be 29d9 f376 2dfc f424 2e48
0000170 f650 30a1 f6fb 3171 f6cf 323c fa51 371d
0000180 0000 3a6c 0659 40ec 04da 4099 029e 3baf
0000190 feb7 387f fc6a 385c 00b7 39d8 05f0 3f54
00001a0 1099 4be4 184b 5524 19c7 5504 1742 51b8
00001b0 10bc 4a26 0966 43fc 01dd 3d63 ff25 387f
00001c0 02d4 3b83 06cb 409c 0e31 4968 0f05 49d0
00001d0 0bd2 4722 0533 4086 fca7 36c7 f931 31f6
```

In the above example the signal level is ok. Here how it looks when the signal is weak:


```
$ arecord -f cd -t raw -d 1 | hexdump | head -n 30
Recording raw data 'stdin' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo
0000000 fc09 fc9d fc0b fc9f fc09 fc9d fc0b fc9c
0000010 fc0a fc9d fc09 fc9d fc0b fc9c fc09 fc9b
0000020 fc0b fc9e fc09 fc9d fc0a fc9d fc0a fc9d
0000030 fc0a fc9d fc0b fc9e fc09 fc9c fc09 fc9b
0000040 fc0b fc9e fc0a fc9c fc0a fc9d fc0a fc9c
0000050 fc09 fc9c fc0b fc9d fc0a fc9b fc0b fc9e
0000060 fc0b fc9d fc0b fc9d fc0c fc9f fc0b fc9d
0000070 fc0d fc9f fc0d fc9d fc0d fc9f fc0d fc9e
0000080 fc0b fc9d fc0d fca0 fc0b fc9d fc0c fc9f
0000090 fc0b fc9e fc0d fc9f fc0c fc9f fc0b fc9d
00000a0 fc0d fc9d fc08 fc9c fc0d fc9d fc0a fc9c
00000b0 fc0a fc9d fc0c fc9d fc09 fc9c fc0c fc9d
00000c0 fc0b fc9c fc0b fc9d fc0a fc9c fc0a fc9d
00000d0 fc0c fc9e fc0b fc9d fc0b fc9d fc0a fc9d
00000e0 fc0a fc9b fc09 fc9b fc0b fc9d fc0b fc9d
00000f0 fc0b fc9d fc0b fc9e fc0b fc9d fc0c fc9e
0000100 fc0d fc9f fc0d fc9e fc0e fca0 fc0e fca1
0000110 fc0d fc9f fc0d fc9f fc0c fc9e fc0b fc9f
0000120 fc0b fc9d fc0b fc9d fc0c fc9f fc0b fc9d
0000130 fc0d fc9f fc0b fc9d fc0b fc9e fc0b fc9d
0000140 fc0a fc9d fc0d fc9e fc0a fc9c fc0c fc9e
0000150 fc0b fc9d fc0a fc9d fc0c fc9d fc09 fc9d
0000160 fc0c fc9d fc0a fc9d fc0b fc9d fc08 fc9d
0000170 fc0a fc9c fc0b fc9d fc08 fc9c fc0b fc9d
0000180 fc0b fc9d fc0c fc9d fc0b fc9d fc0c fc9f
0000190 fc0f fca1 fc0b fc9f fc0f fca0 fc0c fc9f
00001a0 fc0f fca0 fc0e fca0 fc0b fc9f fc0e fca1
00001b0 fc0c fc9d fc0c fc9d fc0b fc9f fc0d fc9e
00001c0 fc0c fc9f fc0b fc9e fc0c fc9e fc0c fc9e
00001d0 fc0b fc9d fc0b fc9d fc0a fc9d fc0b fc9e
```

When the signal is zero:

```
$ arecord -f cd -t raw -d 1 | hexdump | head -n 30
Recording raw data 'stdin' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo
0000000 0000 0000 0000 0000 0000 0000 0000 0000
*
002b110
```

## Randomness mixer: randomness from the sound input and the mouse

The script `randomness_mixer.py` creates a BIP39 mnemonic by collecting randomness from the mouse, the sound input and the `haveged`. Make sure you have `arecord` and `haveged` installed on your system. The script also uses random data entered by the user via the keyboard.

In case the script is started without [X](https://en.wikipedia.org/wiki/X_Window_System) (in the virtual terminal) it will read the mouse movements directly from the `/dev/input/mice`. However, it needs to be started as *root* to do this (only the *root* have permission to access `/dev/input/mice`).

The program `arecord` will not work in virtual terminal if the script is started as *root* and the [X](https://en.wikipedia.org/wiki/X_Window_System) is running. But this problem can be avoided simply by running the script in the graphical environment (terminal emulator like `xterm`, `gnome-terminal`) or by shutting down the [X](https://en.wikipedia.org/wiki/X_Window_System).

## Random choice

The script `randomness_choice.py` creates random numbers in a range. It uses randomness from the sound input (`arecord`), `haveged`, random data entered by the user via the keyboard, and from the mouse movements. If called with `nomouse` parameter it does not use the mouse as a randomness source.

The script `randomness_choice.py` requires `randomness_mixer.py` to be in the same directory.

## Random words from a dictionary

The script `random-words.py` chooses random words from a dictionary. It depends on `randomness_choice.py` and `randomness_mixer.py` (must be in the same directory).

If no parameter is specified the script is searching for a dictionary from this hardcoded list (uses the first file that exists):

```
filename_list = [
    "wordlist.txt",
    "/usr/share/keepassxc/wordlists/eff_large.wordlist",
    "/usr/share/dict/american-english",
    "/usr/share/dict/british-english",
    "/usr/share/dict/cracklib-small",
]
```

The script may ask more than once for a random data from the user (if big number of words is specified). In most cases asking for 15 words will require only one call of `refill_global_accumulator()`.

## Dependencies

You may need to install these Python 3 modules (if they are not already installed):

```
$ pip3 install -r requirements.txt
```

To install `arecord` and `haveged`:

```
$ sudo apt-get install alsa-utils haveged
```

