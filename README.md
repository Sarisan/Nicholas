# Nicholas

Nicholas is a Telegram Bot for Sankaku Channel. Inline mode supports post,
book and tag search, also it supports page selection and many other options.
You can continue your search by clicking second button of inline result, it
will put the whole search query with page and options to inline mode. There
are also commands to get original file or information of post/book. In
information message you can click `Tags` button to see post/book tags. You
can also create inline mode shortcuts with `short` command.

## Requirements

* **Telegram Bot API:** [git](https://github.com/tdlib/telegram-bot-api),
alternatively you can use [web](https://api.telegram.org)
* **C11 standard:** full support
* **curl library:** libcurl4-openssl-dev (Debian), curl (Arch Linux)
* **json-c library:** libjson-c-dev (Debian), json-c (Arch Linux)
* **CMake:** 3.10 or newer

## Compiling

```
git clone https://github.com/Sarisan/Nicholas.git
cd Nicholas
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
```

Remember to change `$HOME/.local` to path where your local bin directory
located, alternatively set it to `build`

```
cmake --build build --target install -j 4
```

## Running

```
nicholas --help
```
If you set path or
```
./build/bin/nicholas --help
```
If you set it to `build`
