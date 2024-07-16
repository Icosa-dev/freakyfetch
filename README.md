# Freakyfetch

![screenshot](screenshot.png)

Finaly, a fetch that can match your freak. Freakyfetch is
a fork of uwufetch that aims to make your OS more 𝓯𝓻𝓮𝓪𝓴𝔂.

## Installation

### Requirments

- [freecolor](http://www.rkeene.org/oss/freecolor/) to get ram usage on FreeBSD.

- [musl libc](https://musl.libc.org/) if you are on a non-gnu system (such as [alpine linux](https://pkgs.alpinelinux.org/package/edge/main/x86_64/musl-dev))

- [xwininfo](https://github.com/freedesktop/xorg-xwininfo) to get screen resolution.

- [viu](https://github.com/atanunq/viu) (optional) to use images instead of ascii art (see [How to use images](#how-to-use-images) below).

- [lshw](https://github.com/lyonel/lshw) (optional) for better accuracy on GPU info.

### Building from source

To install Freakyfetch from the source, type these commands in the terminal:

```shell
git clone https://github.com/H4drian/Freakyfetch
cd Freakyfetch
make build # add "CFLAGS+=-D__IPHONE__" if you are building for iOS
sudo make install
```

To uninstall:

```shell
cd Freakyfetch
sudo make uninstall
```

#### Available Make targets

```shell
make build              # builds freakyfetch and libfetch
make lib                # builds only libfetch
make debug              # use for debug
make install            # installs freakyfetch (needs root permissons)
make uninstall          # uninstalls freakyfetch (needs root permissons)
make clean              # removes all build output
make man                # compiles man page
make man_debug          # compiles man page and shows 'man' output
```
