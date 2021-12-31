# Nicholas

Nicholas is a Telegram bot for searching on Sankaku Channel via inline mode.

## Dependencies

* [Telegram Bot API server](https://github.com/tdlib/telegram-bot-api)
* curl
* json-c
* clang
* lld

## Build and run

Compile the bot with `clang *.c -O3 -fuse-ld=lld -flto=thin -Wl,-O3,--lto-O3 -Wall -lcurl -ljson-c -lpthread -Iinclude -o nicholas` in the root of repo and enter `./nicholas` to start the bot. You can also compile the bot in VS Code.
